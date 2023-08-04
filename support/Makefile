OBJS	= 	geo_math.o json.o json_utils.o utils.o vec_math.o \
			actor.o engine.o engine_assets.o scene_system.o \
			gui_buffer.o gui_font.o gui_label.o gui_system.o \
			gui_view.o input_system.o nav.o nav_mesh.o nav_system.o \
			part_system.o hint.o material.o renderer.o render_system.o \
			skel.o skel_anim.o skel_model.o skel_skin.o static_mesh.o \
			static_model.o texture.o script.o script_system.o snd.o \
			snd_driver.o snd_system.o gl_3.o gl_prog.o main_sdl.o
SOURCE	= 	geo_math.c json.c json_utils.c utils.c vec_math.c \
			actor.c engine.c engine_assets.c scene_system.c \
			gui_buffer.c gui_font.c gui_label.c gui_system.c \
			gui_view.c input_system.c nav.c nav_mesh.c nav_system.c \
			part_system.c hint.c material.c renderer.c render_system.c \
			skel.c skel_anim.c skel_model.c skel_skin.c static_mesh.c \
			static_model.c texture.c script.c script_system.c snd.c \
			snd_driver.c snd_system.c gl_3.c gl_prog.c main_sdl.c
HEADER	=
CC	 = gcc
FLAGS	 = -g -c -Wall $(SDL_CFLAGS)
LFLAGS	 =
INC =	-I$(CORE) -I$(GAME) -I$(GUI) -I$(INPUT)		\
		-I$(NAV) -I$(PART) -I$(RENDER) -I$(SCRIPT)	\
		-I$(SOUND) -I$(GL_3) -I$(GLAD) -I$(SDL)		\
		-I$(PLATFORM)
		
ifeq ($(OS),Windows_NT)
OUT	= ../prb.exe
NIX_LIB =
else
OUT	= ../prb
NIX_LIB = -lm -ldl
endif
		
SDL_CFLAGS = `sdl2-config --cflags`
SDL_LIBS = `sdl2-config --libs`

ENGINE = ../source/engine/
CORE = $(ENGINE)core/
GAME = $(ENGINE)game/
GUI = $(ENGINE)gui/
INPUT = $(ENGINE)input/
NAV = $(ENGINE)nav/
PART = $(ENGINE)part/
RENDER = $(ENGINE)render/
SCRIPT = $(ENGINE)script/
SOUND = $(ENGINE)sound/

PLATFORM = ../source/platform/
GL_3 = $(PLATFORM)gl_3/
GLAD = $(PLATFORM)glad/
SDL = $(PLATFORM)sdl/

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS) $(SDL_LIBS) $(NIX_LIB)

geo_math.o: $(CORE)geo_math.c
	$(CC) $(FLAGS) $(INC) $(CORE)geo_math.c 

json.o: $(CORE)json.c
	$(CC) $(FLAGS) $(INC) $(CORE)json.c 

json_utils.o: $(CORE)json_utils.c
	$(CC) $(FLAGS) $(INC) $(CORE)json_utils.c 

utils.o: $(CORE)utils.c
	$(CC) $(FLAGS) $(CORE)utils.c 

vec_math.o: $(CORE)vec_math.c
	$(CC) $(FLAGS) $(INC) $(CORE)vec_math.c 

actor.o: $(GAME)actor.c
	$(CC) $(FLAGS) $(INC)  $(GAME)actor.c 

engine.o: $(GAME)engine.c
	$(CC) $(FLAGS) $(INC) $(GAME)engine.c 

engine_assets.o: $(GAME)engine_assets.c
	$(CC) $(FLAGS) $(INC) $(GAME)engine_assets.c 

scene_system.o: $(GAME)scene_system.c
	$(CC) $(FLAGS) $(INC) $(GAME)scene_system.c 

gui_buffer.o: $(GUI)gui_buffer.c
	$(CC) $(FLAGS) $(INC) $(GUI)gui_buffer.c 

gui_font.o: $(GUI)gui_font.c
	$(CC) $(FLAGS) $(INC) $(GUI)gui_font.c 

gui_label.o: $(GUI)gui_label.c
	$(CC) $(FLAGS) $(INC) $(GUI)gui_label.c 

gui_system.o: $(GUI)gui_system.c
	$(CC) $(FLAGS) $(INC) $(GUI)gui_system.c 

gui_view.o: $(GUI)gui_view.c
	$(CC) $(FLAGS) $(INC) $(GUI)gui_view.c 

input_system.o: $(INPUT)input_system.c
	$(CC) $(FLAGS) $(INC) $(INPUT)input_system.c 

nav.o: $(NAV)nav.c
	$(CC) $(FLAGS) $(INC) $(NAV)nav.c 

nav_mesh.o: $(NAV)nav_mesh.c
	$(CC) $(FLAGS) $(INC) $(NAV)nav_mesh.c 

nav_system.o: $(NAV)nav_system.c
	$(CC) $(FLAGS) $(INC) $(NAV)nav_system.c 

part_system.o: $(PART)part_system.c
	$(CC) $(FLAGS) $(INC) $(PART)part_system.c 

hint.o: $(RENDER)hint.c
	$(CC) $(FLAGS) $(INC) $(RENDER)hint.c 

material.o: $(RENDER)material.c
	$(CC) $(FLAGS) $(INC) $(RENDER)material.c 

renderer.o: $(RENDER)renderer.c
	$(CC) $(FLAGS) $(INC) $(RENDER)renderer.c 

render_system.o: $(RENDER)render_system.c
	$(CC) $(FLAGS) $(INC) $(RENDER)render_system.c 

skel.o: $(RENDER)skel.c
	$(CC) $(FLAGS) $(INC) $(RENDER)skel.c 

skel_anim.o: $(RENDER)skel_anim.c
	$(CC) $(FLAGS) $(INC) $(RENDER)skel_anim.c 

skel_model.o: $(RENDER)skel_model.c
	$(CC) $(FLAGS) $(INC) $(RENDER)skel_model.c 

skel_skin.o: $(RENDER)skel_skin.c
	$(CC) $(FLAGS) $(INC) $(RENDER)skel_skin.c 

static_mesh.o: $(RENDER)static_mesh.c
	$(CC) $(FLAGS) $(INC) $(RENDER)static_mesh.c 

static_model.o: $(RENDER)static_model.c
	$(CC) $(FLAGS) $(INC) $(RENDER)static_model.c 

texture.o: $(RENDER)texture.c
	$(CC) $(FLAGS) $(INC) $(RENDER)texture.c 

script.o: $(SCRIPT)script.c
	$(CC) $(FLAGS) $(INC) $(SCRIPT)script.c 

script_system.o: $(SCRIPT)script_system.c
	$(CC) $(FLAGS) $(INC) $(SCRIPT)script_system.c 

snd.o: $(SOUND)snd.c
	$(CC) $(FLAGS) $(INC) $(SOUND)snd.c 

snd_driver.o: $(SOUND)snd_driver.c
	$(CC) $(FLAGS) $(INC) $(SOUND)snd_driver.c 

snd_system.o: $(SOUND)snd_system.c
	$(CC) $(FLAGS) $(INC) $(SOUND)snd_system.c 

gl_3.o: $(GL_3)gl_3.c
	$(CC) $(FLAGS) $(INC) $(GL_3)gl_3.c 

gl_prog.o: $(GL_3)gl_prog.c
	$(CC) $(FLAGS) $(INC) $(GL_3)gl_prog.c 

main_sdl.o: $(SDL)main_sdl.c
	$(CC) $(FLAGS) $(INC) $(SDL)main_sdl.c

clean:
	rm -f $(OBJS) $(OUT)
