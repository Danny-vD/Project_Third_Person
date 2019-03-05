#include <iostream>
#include <string>

#include "glm.hpp"

#include "mge/core/Renderer.hpp"

#include "mge/core/Mesh.hpp"
#include "mge/core/World.hpp"
#include "mge/core/Texture.hpp"
#include "mge/core/Light.hpp"
#include "mge/core/Camera.hpp"
#include "mge/core/GameObject.hpp"

#include "mge/materials/AbstractMaterial.hpp"
#include "mge/materials/ColorMaterial.hpp"
#include "mge/materials/TextureMaterial.hpp"
#include "mge/materials/LitMaterial.hpp"

#include "mge/behaviours/RotatingBehaviour.hpp"
#include "mge/behaviours/KeysBehaviour.hpp"
#include "mge/behaviours/MouseRotatingBehaviour.hpp"
#include "mge/behaviours/EmptyBehaviour.hpp"

#include "mge/util/DebugHud.hpp"

#include "ThirdPerson/config.hpp"
#include "ThirdPerson/TPerson.hpp"
#include "ThirdPerson/Ray.hpp"
#include "ThirdPerson/RenderToTexture.hpp"

std::vector<GameObject*> TPerson::puzzleObjects = std::vector<GameObject*>();

//construct the game class into _window, _renderer and hud (other parts are initialized by build)
TPerson::TPerson() :AbstractGame(), _hud(0)
{

}

void TPerson::initialize()
{
	TPerson::_levelIndex = 1;
	//setup the core part
	AbstractGame::initialize();

	//setup the custom part so we can display some text
	std::cout << "Initializing HUD" << std::endl;
	_hud = new DebugHud(_window);
	std::cout << "HUD initialized." << std::endl << std::endl;
}

//build the game _world
void TPerson::_initializeScene()
{
	//MESHES
	this->renderToTexture = new RenderToTexture();
	//load a bunch of meshes we will be using throughout this demo
	//each mesh only has to be loaded once, but can be used multiple times:
	//F is flat shaded, S is smooth shaded (normals aligned or not), check the models folder!
	Mesh* planeMeshDefault = Mesh::load(config::THIRDPERSON_MODEL_PATH + "plane.obj");
	Mesh* sphereMesh = Mesh::load(config::THIRDPERSON_MODEL_PATH + "sphere_smooth.obj");
	Mesh* umbrellaMesh = Mesh::load(config::THIRDPERSON_MODEL_PATH + "Umbrella.obj");
	Mesh* pliersDown = Mesh::load(config::THIRDPERSON_MODEL_PATH + "PliersDown.obj");
	Mesh* pliersUp = Mesh::load(config::THIRDPERSON_MODEL_PATH + "PliersUp.obj");
	Mesh* plunger1 = Mesh::load(config::THIRDPERSON_MODEL_PATH + "Toilet_plunger_1.obj");
	Mesh* plunger2 = Mesh::load(config::THIRDPERSON_MODEL_PATH + "Toilet_plunger_2.obj");
	Mesh* syringeMesh = Mesh::load(config::THIRDPERSON_MODEL_PATH + "Syringe.obj");
	Mesh* scissorMesh = Mesh::load(config::THIRDPERSON_MODEL_PATH + "Scissor.obj");
	Mesh* bottleMesh = Mesh::load(config::THIRDPERSON_MODEL_PATH + "Bottle.obj");
	Mesh* deskMesh = Mesh::load(config::THIRDPERSON_MODEL_PATH + "Desk.obj");

	//MATERIALS

	//create some materials to display the cube, the plane and the light
	AbstractMaterial* lightMaterial = new ColorMaterial(glm::vec3(1, 1, 0));
	AbstractMaterial* runicStoneMaterial = new TextureMaterial(Texture::load(config::THIRDPERSON_TEXTURE_PATH + "bricks.jpg"));
	AbstractMaterial* landMaterial = new TextureMaterial(Texture::load(config::THIRDPERSON_TEXTURE_PATH + "land.jpg"));
	AbstractMaterial* umbrellaMaterial = new TextureMaterial(Texture::load(config::THIRDPERSON_TEXTURE_PATH + "AlbedoUmbrella.png"));
	litMaterialR = new LitMaterial(glm::vec3(1, 0, 0));
	litMaterialG = new LitMaterial(glm::vec3(0.5f, 0.5f, 0.5f));
	litMaterialB = new LitMaterial(glm::vec3(0, 0, 1));
	blackMaterial = new ColorMaterial(glm::vec3(0, 0, 0));
	greyMaterial = new ColorMaterial(glm::vec3(0.5f, 0.5f, 0.5f));
	shadowMaterial = new TextureMaterial(renderToTexture->getTexture());

	//SCENE SETUP

   //add camera first (it will be updated last)
	camera = new Camera("camera", glm::vec3(0, 3, 2.5f));
	camera->rotate(glm::radians(-45.0f), glm::vec3(1, 0, 0));
	camera->setBehaviour(new KeysBehaviour(25, 90));
	_world->add(camera);
	_world->setMainCamera(camera);

	//add a light. Note that the light ABSOLUTELY WORKS! YES ! REALLY !

	room = new Room(this, _world, _window, _levelIndex);
	_world->add(room);

	//a light to light the scene!
	light = new Light("light", glm::vec3(0, 4.0f, 0), LightType::POINT);
	light->scale(glm::vec3(0.1f, 0.1f, 0.1f));
	light->rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0));
	light->translate(glm::vec3(0, 0, 3));
	light->SetLightIntensity(1.5f);
	light->setMesh(sphereMesh);
	light->setMaterial(lightMaterial);
	//light->setBehaviour(new KeysBehaviour(25, 90));
	_world->add(light);
	//light->SetLightColor(glm::vec3(1, 0, 0.8f));
	LitMaterial::AddLight(light);

	//add the plane
	plane = new GameObject("plane", glm::vec3(-0, 0.9f, -0));
	plane->scale(glm::vec3(1, 1, 1));
	/*plane->rotate(glm::radians(-90.0f), glm::vec3(0, 0, 1));
	plane->rotate(glm::radians(25.0f), glm::vec3(1, 0, 0));
	plane->rotate(glm::radians(90.0f), glm::vec3(0, 1, 0));*/
	plane->setMesh(planeMeshDefault);
	plane->setMaterial(shadowMaterial);
	_world->add(plane);

	//add the floor
	GameObject* floor = new GameObject("floor", glm::vec3(0, -2.5f, 0));
	floor->scale(glm::vec3(15, 15, 15));
	floor->setMesh(planeMeshDefault);
	floor->setMaterial(greyMaterial);
	_world->add(floor);

	//add a desk
	desk = new GameObject("desk", glm::vec3(0, 0, 0));
	desk->scale(glm::vec3(3.5f, 3.5f, 3.5f));
	//desk->rotate(glm::radians(45.0f), glm::vec3(0, 1, 0));
	desk->setMesh(deskMesh);
	desk->setMaterial(litMaterialB);
	_world->add(desk);

	//add a puzzle object
	//puzzleObject = new GameObject("puzzleObject", glm::vec3(0, 3, 0));
	//puzzleObject->scale(glm::vec3(0.5f, 0.5f, 0.5f));
	////puzzleObject->rotate(glm::radians(45.0f), glm::vec3(1, 0.5f, 0));
	//puzzleObject->setMesh(umbrellaMesh);
	//puzzleObject->setMaterial(litMaterialR);
	//puzzleObject->setBehaviour(new MouseRotatingBehaviour(_window, _world));
	//_world->add(puzzleObject);

	puzzleObject1 = new GameObject("puzzleObject1", glm::vec3(0, 2, 0));
	puzzleObject1->scale(glm::vec3(0.1, 0.1, 0.1));
	puzzleObject1->rotate(glm::radians(95.0f), glm::vec3(1, 0.5f, 0.6f));
	puzzleObject1->setMesh(bottleMesh);
	puzzleObject1->setMaterial(umbrellaMaterial);
	puzzleObject1->setBehaviour(new MouseRotatingBehaviour(_window, _world));
	_world->add(puzzleObject1);

	//puzzleObject2 = new GameObject("puzzleObject2", glm::vec3(-0.23, 1.785, 0.1));
	//puzzleObject2->scale(glm::vec3(0.1, 0.1, 0.1));
	//puzzleObject2->rotate(glm::radians(45.0f), glm::vec3(1, 0.5f, 0));
	//puzzleObject2->setMesh(plunger2);
	//puzzleObject2->setMaterial(litMaterialR);
	//puzzleObject2->setBehaviour(new MouseRotatingBehaviour(_window, _world));
	//_world->add(puzzleObject2);

	//puzzleObjects.push_back(puzzleObject);
	puzzleObjects.push_back(puzzleObject1);
	//puzzleObjects.push_back(puzzleObject2);
}

void TPerson::_render()
{
	glm::mat4 transform = camera->getTransform();
	renderToTexture->bindFramebuffer();
	glClearColor(0.5f, 0.5f, 0.5f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera->setTransform(light->getTransform());
	camera->SetFOV(30.0f);
	//puzzleObject->setMaterial(blackMaterial);
	puzzleObject1->setMaterial(blackMaterial);
	//puzzleObject2->setMaterial(blackMaterial);
	desk->setMaterial(litMaterialB);
	plane->setMaterial(greyMaterial);
	AbstractGame::_render();
	renderToTexture->unbindFramebuffer();

	//puzzleObject->setMaterial(litMaterialR);
	puzzleObject1->setMaterial(litMaterialR);
	//puzzleObject2->setMaterial(litMaterialR);
	desk->setMaterial(litMaterialB);
	plane->setMaterial(shadowMaterial);
	camera->setTransform(transform);
	camera->SetFOV(60.0f);
	glClearColor(0.2f, 0.2f, 0.2f, 1);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	AbstractGame::_render();
	_updateHud();

	if (puzzleObjects.size() == 1)
	{
		_checkOnePuzzle();
	}


	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		puzzleObject1->setWorldRotation(glm::vec3(0, 0, 0));
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
	{
		puzzleObject1->rotate(glm::radians(1.0f), glm::vec3(1, 0, 0));
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
	{
		puzzleObject1->rotate(glm::radians(1.0f), glm::vec3(0, 1, 0));
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::C))
	{
		puzzleObject1->rotate(glm::radians(1.0f), glm::vec3(0, 0, 1));
	}
}

void TPerson::_updateHud()
{
	std::string debugInfo = "";
	debugInfo += std::string("FPS:") + std::to_string((int)_fps) + "\n";

	_hud->setDebugInfo(debugInfo);
	_hud->draw();
}

void TPerson::_checkOnePuzzle()
{
	glm::vec3 rotation = puzzleObject1->getWorldRotation();// -glm::vec3(solutionDegreesX, solutionDegreesY, solutionDegreesZ);
	float tolerance = glm::length(rotation);

	//std::cout << tolerance << std::endl;
	//std::cout << solutionDegreesX << ", ";
	//std::cout << solutionDegreesY << ", ";
	//std::cout << solutionDegreesZ << std::endl;
	std::cout << rotation.x << ", ";
	std::cout << rotation.y << ", ";
	std::cout << rotation.z << std::endl;

	if (rotation.y <= 10 || rotation.y >= 170)
	{
		std::cout << "BITCH LASAGNA" << std::endl;
		//puzzleObject1->setBehaviour(new EmptyBehaviour());

		completed = true;
	}

	if (completed)
	{
		//do stuff	
		//puzzleObject1->setWorldRotation(glm::vec3(0, 0, 0));
	}
}

void TPerson::MoveToPreviousLevel()
{
	_levelIndex--;
	if (_levelIndex < 1) { _levelIndex = 1; return; }
	_world->remove(room);
	delete(room);
	room = new Room(this, _world, _window, _levelIndex);
	_world->add(room);
}

void TPerson::MoveToNextLevel()
{
	_levelIndex++;
	if (_levelIndex > 2) { _levelIndex = 2; return; }
	_world->remove(room);
	delete(room);
	room = new Room(this, _world, _window, _levelIndex);
	_world->add(room);
}

TPerson::~TPerson()
{
	//dtor
}

