#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint area_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > area_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    MeshBuffer const *ret = new MeshBuffer(data_path("area.pnct"));
    area_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
    return ret;
});

Load< Scene > area_scene(LoadTagDefault, []() -> Scene const * {
    return new Scene(data_path("area.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
        Mesh const &mesh = area_meshes->lookup(mesh_name);

        scene.drawables.emplace_back(transform);
        Scene::Drawable &drawable = scene.drawables.back();

        drawable.pipeline = lit_color_texture_program_pipeline;

        drawable.pipeline.vao = area_meshes_for_lit_color_texture_program;
        drawable.pipeline.type = mesh.type;
        drawable.pipeline.start = mesh.start;
        drawable.pipeline.count = mesh.count;

    });
});

Load< Sound::Sample > main_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("gameaudio.wav"));
});

Load< Sound::Sample > timeout_sample(LoadTagDefault, []() -> Sound::Sample const * {
    return new Sound::Sample(data_path("timedout.wav"));
});


PlayMode::PlayMode() : scene(*area_scene) {
    for (auto &transform : scene.transforms) {

        if (transform.name == "Cube") cube0 = &transform;
        else if (transform.name == "Cube.001") cube1 = &transform;
    }

	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

    cube0->position.x=0;
    cube0->position.y=0;

    float randomx = cube0->position.x;
    float randomy = cube0->position.y;
    while (std::abs(randomy-cube0->position.y)<=10.0) {
        randomx = ((float) rand()) / (float) RAND_MAX;
        randomx = (randomx*75) - 37.5;
        randomy = ((float) rand()) / (float) RAND_MAX;
        randomy = (randomy*75) - 37.5;
    }
    cube1->position.x = randomx;
    cube1->position.y = randomy;

    camera->transform->position.y = -49.0f;
    camera->transform->position.z = 185.0f;

    main_sound_loop = Sound::loop_3D(*main_sample, 20.0f, get_main_sound_position(), 0.1f);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

    if (evt.type == SDL_KEYDOWN) {
        if (evt.key.keysym.sym == SDLK_a) {
            ak.downs += 1;
            ak.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_d) {
            dk.downs += 1;
            dk.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_w) {
            wk.downs += 1;
            wk.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_s) {
            sk.downs += 1;
            sk.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_LEFT) {
            left.downs += 1;
            left.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_RIGHT) {
            right.downs += 1;
            right.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_UP) {
            up.downs += 1;
            up.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_DOWN) {
            down.downs += 1;
            down.pressed = true;
            return true;
        }
    } else if (evt.type == SDL_KEYUP) {
        if (evt.key.keysym.sym == SDLK_a) {
            ak.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_d) {
            dk.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_w) {
            wk.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_s) {
            sk.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_LEFT) {
            left.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_RIGHT) {
            right.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_UP) {
            up.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_DOWN) {
            down.pressed = false;
            return true;
        }
    }

	return false;
}

void PlayMode::update(float elapsed) {
    if (found>0) found-=elapsed;
    if (dead - elapsed > 0) {
        dead -= elapsed;
        main_sound_loop->set_position(camera->transform->position, 1.0f);
        return;
    } else if (dead>0) {
        dead = 0;
        timer = 90.0f;
        round = 0;
        cube0->position.x = 0;
        cube0->position.y = 0;
        float randomx = cube0->position.x;
        float randomy = cube0->position.y;
        while (std::abs(randomy-cube0->position.y)<=10.0) {
            randomx = ((float) rand()) / (float) RAND_MAX;
            randomx = (randomx*75) - 37.5;
            randomy = ((float) rand()) / (float) RAND_MAX;
            randomy = (randomy*75) - 37.5;
        }
        cube1->position.x = randomx;
        cube1->position.y = randomy;

        main_sound_loop = Sound::loop_3D(*main_sample, 20.0f, get_main_sound_position(), 0.1f);
    } else {
        timer-=elapsed;
    }

    if (timer<=0) {
        dead = 3.0f;
        best_score = std::max(best_score, round);
        main_sound_loop->stop();
        timedout = Sound::play(*timeout_sample, 1.0f, 3.0f);
    }

    if (std::abs(cube0->position.x-cube1->position.x)<=5 && std::abs(cube0->position.y-cube1->position.y)<=5){
        round++;
        found=2.0f;
        float randomx = cube0->position.x;
        float randomy = cube0->position.y;
        while (std::abs(randomy-cube0->position.y)<=10.0) {
            randomx = ((float) rand()) / (float) RAND_MAX;
            randomx = (randomx*75) - 37.5;
            randomy = ((float) rand()) / (float) RAND_MAX;
            randomy = (randomy*75) - 37.5;
        }
        cube1->position.x = randomx;
        cube1->position.y = randomy;
    }


    if (right.pressed||left.pressed||down.pressed||up.pressed) speed += elapsed / 10.0f;
    else speed = elapsed;


    if (right.pressed) {
        cube0->position.x+=speed;
    }
    if (left.pressed) {
        cube0->position.x-=speed;
    }
    if (down.pressed) {
        cube0->position.y-=speed;
    }
    if (up.pressed) {
        cube0->position.y+=speed;
    }

    main_sound_loop->set_position(get_main_sound_position(), 1.0f);


    { //update listener to camera position:
        glm::mat4x3 frame = camera->transform->make_local_to_parent();
        glm::vec3 right = frame[0];
        glm::vec3 at = frame[3];
        Sound::listener.set_position_right(at, right, 1.0f);
    }

    //reset button press counters:
    left.downs = 0;
    right.downs = 0;
    up.downs = 0;
    down.downs = 0;
    wk.downs = 0;
    ak.downs = 0;
    sk.downs = 0;
    dk.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));


        constexpr float H = 0.09f;

        std::string s4 = "found one! On to another!";
        std::string s = "time left: " + std::to_string(timer);
        std::string s1 = "boxes found: " + std::to_string(round);
        std::string s2 = "best score: " + std::to_string(best_score);

        std::string s3 = "Timed out";
        if (dead>0)lines.draw_text(s3,
                                   glm::vec3(0, 0, 0.0),
                                   glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                                   glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        if (found>0)lines.draw_text(s4,
                                   glm::vec3(-0.4f, -0.1f, 0.0),
                                   glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                                   glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        lines.draw_text(s,
                        glm::vec3(0.8f * H +0.9f, 0.1f * H, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        lines.draw_text(s1,
                        glm::vec3(0.8f * H + 0.9f , 0.1f * H+0.1f, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        lines.draw_text(s2,
                        glm::vec3(0.8f * H + 0.9f , 0.1f * H+0.2f, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0xff, 0xff, 0xff, 0x00));

	}
	GL_ERRORS();
}

glm::vec3 PlayMode::get_main_sound_position() {
    return glm::vec3(cube1->position.x+camera->transform->position.x-cube0->position.x, cube1->position.y+camera->transform->position.y-cube0->position.y, cube1->position.z+camera->transform->position.z);
}
