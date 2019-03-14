#include <SFML/Window/Keyboard.hpp>

#include "mge/core/Renderer.hpp"

#include "mge/core/Mesh.hpp"
#include "mge/core/World.hpp"
#include "mge/core/Texture.hpp"
#include "mge/core/GameObject.hpp"
#include "ThirdPerson/UITexture.hpp"
#include "mge/core/Camera.hpp"

#include "mge/materials/AbstractMaterial.hpp"
#include "mge/materials/ColorMaterial.hpp"
#include "mge/materials/TextureMaterial.hpp"
#include "mge/materials/LitTextureMaterial.hpp"
#include "mge/materials/LitMaterial.hpp"
#include "mge/materials/RenderToTextureMaterial.hpp"
#include "ThirdPerson/buttons/QuitGameButton.hpp"
#include "ThirdPerson/buttons/ResumeGameButton.hpp"
#include "ThirdPerson/buttons/RestartGameButton.hpp"
#include "ThirdPerson/buttons/ReturnToMenuButton.hpp"

#include "mge/behaviours/RotatingBehaviour.hpp"
#include "ThirdPerson/config.hpp"
#include "ThirdPerson/TPerson.hpp"

#include "ThirdPerson/RenderToTexture.hpp"
#include "Room.hpp"

GameObject* _sphere;

Room::Room(TPerson* pGame, World* pWorld, sf::RenderWindow* pWindow, RenderToTexture* pRender, std::string pName, glm::vec3 pPosition)
	: GameObject(pName, pPosition), _renderToTexture(pRender), _window(pWindow), _roomWorld(pWorld)
{
	_game = pGame;
	_blackMaterial = new ColorMaterial(glm::vec3(0, 0, 0));
	_levelIndex = 1;

	loadRoom();
}

void Room::loadRoom()
{
	// add parent object to world
	_roomParent = new GameObject("room", glm::vec3(0, 0, 0));
	_roomWorld->add(_roomParent);

	/* Load LUA */
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	if (luaL_loadfile(L, "../src/ThirdPerson/room.lua") || lua_pcall(L, 0, 0, 0))
	{
		printf("Cannot run file\n");
		return;
	}

	// Print table contents.
	lua_getglobal(L, "objects");

	print_table(L);

	lua_close(L);

	//second light
	Light* AMlight = new Light("AMlight", glm::vec3(0, -4, 0), LightType::DIRECTIONAL);
	AMlight->rotate(glm::radians(180.0f), glm::vec3(0, 1, 0));
	AMlight->rotate(glm::radians(45.0f), glm::vec3(1, 0, 0));

	AMlight->SetLightIntensity(.8f);
	_roomParent->add(AMlight);
	LitMaterial::AddLight(AMlight);

	//a light to light the scene!
	glm::vec3 color(1, 1, 1);
	AbstractMaterial* lightMaterial = new ColorMaterial(color);
	light = new Light("light", glm::vec3(0, 4, .5f), LightType::SPOT); //0, 4, 0
	light->scale(glm::vec3(0.1f, 0.1f, 0.1f));
	light->rotate(glm::radians(90.0f), glm::vec3(1, 0, 0));
	//light->translate(glm::vec3(0, 0, 3));
	light->SetLightIntensity(1.8f);
	light->setAmbientContribution(0.2f);
	//Mesh* mesh = Mesh::load(config::THIRDPERSON_MODEL_PATH + "cone_smooth.obj");
	//light->setMesh(mesh);
	//light->setMaterial(lightMaterial);
	light->SetLightColor(color); //1, 0, 0.8f

	_roomParent->add(light);
	LitMaterial::AddLight(light);

	//Cigar light
	Light* cigarlight = new Light("CigarLight", glm::vec3(1.94f, .9f, 0.7f), LightType::POINT);
	cigarlight->SetLightColor(glm::vec3(0.886f, 0.15f, 0));
	cigarlight->setFalloff(35);
	_roomParent->add(cigarlight);
	LitMaterial::AddLight(cigarlight);

	//Lamp light
	//Light* lamplight = new Light("lampLight", glm::vec3(2.2f, 2.0f, -1.0f), LightType::SPOT);
	//lamplight->rotate(glm::radians(90.0f), glm::vec3(1, 0, 0));
	////mesh = Mesh::load(config::THIRDPERSON_MODEL_PATH + "sphere_smooth.obj");
	//lamplight->SetLightColor(glm::vec3(0.917f, 0.917f, 0.259f));/*
	//lamplight->setMesh(mesh);
	//lamplight->setMaterial(new ColorMaterial(lamplight->GetColor()));
	//lamplight->scale(glm::vec3(0.05f, 0.05f, 0.05f));*/
	//lamplight->SetLightIntensity(.6f);
	//_roomParent->add(lamplight);
	//LitMaterial::AddLight(lamplight);

	// pause menu
	_gameHud = new UserInterface(_window);
	_roomParent->add(_gameHud);
	_gameHud->Paused = true;
	UITexture* pauseMenu = new UITexture(_window, "pausemenu.png");
	pauseMenu->SetPosition(glm::vec2((_window->getSize().x / 2) - (pauseMenu->GetRect().width / 2), (_window->getSize().y / 2) - (pauseMenu->GetRect().height / 2)));
	_gameHud->Add(pauseMenu);
	_gameHud->AddButton(new ResumeGameButton(_window, this, "Continuepause.png", "continuecelectedpause.png", glm::vec2(pauseMenu->GetPosition().x + 150, pauseMenu->GetPosition().y + 250)));
	_gameHud->AddButton(new RestartGameButton(_window, this, "Restartpause.png", "restartselectedpause.png", glm::vec2(pauseMenu->GetPosition().x + 150, pauseMenu->GetPosition().y + 350)));
	_gameHud->AddButton(new ReturnToMenuButton(_window, this, _game, "Quitpausemenu.png", "quitselectedpause.png", glm::vec2(pauseMenu->GetPosition().x + 150, pauseMenu->GetPosition().y + 500)));
}

void Room::Initialize()
{
	// add puzzle
	_puzzle = new Puzzle(_window, _world, _game, this, _levelIndex);
	_roomParent->add(_puzzle);

	_active = true;
}

void Room::Deinitialize()
{
	_roomParent->remove(_puzzle);
	remove(_puzzle);
	delete(_puzzle);
	_active = false;
}

void Room::update(float pStep)
{
	if (!_active)
		return;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
	{
		MoveToNextLevel();
		if (_paused)
			TogglePause();
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
	{
		MoveToPreviousLevel();
		if (_paused)
			TogglePause();
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::U) && _pauseTimer <= 0)
	{
		TogglePause();
	}
	if (_pauseTimer > 0) {
		_pauseTimer -= pStep;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::F11)) {
		Audio* audio = new Audio(SoundType::MUSIC, config::THIRDPERSON_AUDIO_PATH + "aliud.wav"); // I Will do this
		audio->Play();

		_aliud = new GameObject("Aliud Alpha", glm::vec3(0, 1.5f, 0.5f));
		Mesh* aliudpellis = Mesh::load(config::THIRDPERSON_MODEL_PATH + "Aliud.obj");
		LitTextureMaterial* aliudmateriales = new LitTextureMaterial(Texture::load(config::THIRDPERSON_TEXTURE_PATH + "aliud.png"), glm::vec3(0, 0, 0));
		_aliud->setMesh(aliudpellis);
		_aliud->setMaterial(aliudmateriales);
		_aliud->scale(glm::vec3(0.01f, 0.01f, 0.01f));

		GameObject* _aliudtelum = new GameObject("Aliud Telum", glm::vec3(0.95f, 1.2f, 0.5f));
		Mesh* aliudtelumpellis = Mesh::load(config::THIRDPERSON_MODEL_PATH + "Revolver.obj");
		LitTextureMaterial* aliudtelummateriales = new LitTextureMaterial(Texture::load(config::THIRDPERSON_TEXTURE_PATH + "RevolverBase.png"), glm::vec3(0, 0, 0));
		_aliudtelum->setMesh(aliudtelumpellis);
		_aliudtelum->setMaterial(aliudtelummateriales);
		_aliudtelum->scale(glm::vec3(2, 2, 2));
		_aliudtelum->rotate(1.57f, glm::vec3(0, 1, 0));


		remove(_puzzle);
		add(_aliud);
		add(_aliudtelum);
		_aliudTimer = 7;
		_aliudostium = true;
	}

	if (_aliudostium) {
		if (_aliudTimer > 0) {
			_aliudTimer -= pStep;
		}
		else {
			_window->close();
		}
	}

	//TODO: Remove/Replace this
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::T))
	{
		PlayAnimation("poloroid", false);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::G))
	{
		PlayAnimation("poloroid", true);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y))
	{
		PlayAnimation("whiskey", false);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::H))
	{
		PlayAnimation("whiskey", true);
	}
}


void Room::PlayAnimation(std::string pName, bool pReverse)
{
	if (!pReverse)
	{
		_game->GetMainCamera()->getBehaviour()->FollowPath(pName);
	}
	else
	{
		_game->GetMainCamera()->getBehaviour()->FollowReversePath(pName);
	}
}



void Room::print_table(lua_State *L)
{
	lua_pushnil(L);
	std::string params[2][2];
	glm::vec3 vectors[3] = {glm::vec3{1,1,1}, glm::vec3{1,1,1}, glm::vec3{1,1,1}};
	int index = 0;

	while (lua_next(L, -2) != 0)
	{
		if (lua_isstring(L, -1))
		{
			params[index][0] = lua_tostring(L, -2);
			params[index][1] = lua_tostring(L, -1);
			index++;
		}
		else if (lua_isnumber(L, -1)) {
			printf("%s = %d", lua_tostring(L, -2), (int)lua_tonumber(L, -1));
		}
		else if (lua_istable(L, -1)) {
			if ((std::string)lua_tostring(L, -2) == "position")
			{
				glm::vec3* position = fill_vector3(L);
				vectors[0] = *position;
			}
			else if ((std::string)lua_tostring(L, -2) == "scale")
			{
				glm::vec3* scale = fill_vector3(L);
				vectors[1] = *scale;
			}
			else if ((std::string)lua_tostring(L, -2) == "rotation")
			{
				glm::vec3* rotation = fill_vector3(L);
				vectors[2] = *rotation;
			}
			else
			{
				print_table(L);
			}
		}
		lua_pop(L, 1);

	}
	if (params[0][0] != "")
	{
		addObject(params, vectors);
	}
}

glm::vec3* Room::fill_vector3(lua_State *L)
{
	lua_pushnil(L);
	glm::vec3* vector = new glm::vec3();
	int index = 0;

	while (lua_next(L, -2) != 0)
	{
		if (lua_isnumber(L, -1))
		{
			if ((std::string)lua_tostring(L, -2) == "x")
			{
				vector->x = (float)lua_tonumber(L, -1);
			}
			else if ((std::string)lua_tostring(L, -2) == "y")
			{
				vector->y = (float)lua_tonumber(L, -1);
			}
			else if ((std::string)lua_tostring(L, -2) == "z")
			{
				vector->z = (float)lua_tonumber(L, -1);
			}
		}
		lua_pop(L, 1);
		index++;
	}
	return vector;
}

void Room::addObject(std::string pProperties[2][2], glm::vec3 pVectors[3])
{
	int h = 2 + (std::rand() % (5 - 2 + 1));
	GameObject* object = new GameObject("object", pVectors[0]);

	Mesh* mesh = Mesh::load(config::THIRDPERSON_MODEL_PATH + pProperties[0][1]);
	AbstractMaterial* material;
	if (pProperties[1][1] == "shadow")
	{
		material = new RenderToTextureMaterial(_renderToTexture->getTexture()); //Very important
	}
	else
	{
		material = new LitTextureMaterial(Texture::load(config::THIRDPERSON_TEXTURE_PATH + pProperties[1][1]), glm::vec3(0, 0, 0));
	}

	object->setMesh(mesh);
	object->setMaterial(material);
	object->scale(pVectors[1]);
	object->rotate(glm::radians(pVectors[2].x), glm::vec3(1.0f, 0, 0));
	object->rotate(glm::radians(pVectors[2].y), glm::vec3(0, 1.0f, 0));
	object->rotate(glm::radians(pVectors[2].z), glm::vec3(0, 0, 1.0f));
	_roomParent->add(object);
}

void Room::TogglePause()
{
	_pauseTimer = 0.5f;

	_paused = !_paused;
	_puzzle->Paused = !_puzzle->Paused;
	_gameHud->Paused = !_gameHud->Paused;
}

void Room::DisablePause()
{
	_pauseTimer = 0.5f;

	_paused = false;
	_puzzle->Paused = false;
	_gameHud->Paused = true;
}

void Room::LoadLevel(int pLevel, bool pReload)
{
	if (!pReload){
		_levelIndex = pLevel;
	}

	_roomParent->remove(_puzzle);
	delete(_puzzle);
	_puzzle = new Puzzle(_window, _world, _game, this, _levelIndex);
	_world->add(_puzzle);
}

void Room::MoveToPreviousLevel()
{
	_levelIndex--;
	if (_levelIndex < 1) { _levelIndex = 1; return; }
	_roomParent->remove(_puzzle);
	delete(_puzzle);
	_puzzle = new Puzzle(_window, _world, _game, this, _levelIndex);
	_world->add(_puzzle);

	saveLevel();
}

void Room::MoveToNextLevel()
{
	_levelIndex++;
	if (_levelIndex > 10) { _levelIndex = 10; return; }
	_roomParent->remove(_puzzle);
	delete(_puzzle);
	_puzzle = new Puzzle(_window, _world, _game, this, _levelIndex);
	_roomParent->add(_puzzle);

	saveLevel();
}

void Room::saveLevel()
{
	int level;

	// check if a new higher level has been reached and save it
	std::string line;
	std::ifstream myfile("save.txt");
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			std::cout << line;
			level = std::stoi(line);
		}
		myfile.close();
	}

	if (_levelIndex > level) {
		std::ofstream savefile;
		savefile.open("save.txt", std::fstream::in | std::fstream::trunc);
		savefile << _levelIndex;
		savefile.close();
	}
}

void Room::_render()
{
	if (!_active)
		return;

	glm::mat4 lightTransform = light->getWorldTransform();
	_renderToTexture->Render(_puzzle->getObjects(), _blackMaterial, lightTransform);

	_puzzle->draw();
	_puzzle->PuzzleTimer->draw();
	if (_paused) {
		_gameHud->draw();
	}
}


Room::~Room()
{
	//_world->remove(_sphere);
	if (_world != NULL) {
		_world->remove(_roomParent);
	}
}
