#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, wk, sk, dk, ak;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

    Scene::Transform* cube0;
    Scene::Transform* cube1;
    float speed = 0.0f;
    float enlarge = 0.5f;
    float timer = 90.0f;
    int best_score = 0;
    float dead = 0.0f;
    float found = 0.0f;
    int round = 0;

	glm::vec3 get_main_sound_position();

	std::shared_ptr< Sound::PlayingSample > main_sound_loop;
    std::shared_ptr< Sound::PlayingSample > timedout;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
