.PHONY : all

#n.b. the '-y' sets autoexec scripts to 'on' so that driver expressions will work
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	BLENDER = /Applications/Blender.app/Contents/MacOS/blender -y
else ifeq ($(UNAME_S),Linux)
	BLENDER = ../../blender-2.90.0-linux64/blender -y
else
	BLENDER = blender
endif

EXPORT_MESHES=export-meshes.py
EXPORT_SCENE=export-scene.py

DIST=../dist

all : \
	$(DIST)/area.pnct \
	$(DIST)/area.scene \


$(DIST)/area.scene : area.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Main '$@'

$(DIST)/area.pnct : area.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Main '$@'
