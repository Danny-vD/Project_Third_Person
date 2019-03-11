#include <cassert>
#include <iostream>

#include <GL/glew.h>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Audio/Music.hpp>
#include "ThirdPerson/UserInterface.hpp"
#include "ThirdPerson/buttons/NewGameButton.hpp"
#include "ThirdPerson/buttons/QuitGameButton.hpp"
#include "ThirdPerson/buttons/LevelSelectButton.hpp"
#include "ThirdPerson/buttons/ContinueButton.hpp"
#include "ThirdPerson/config.hpp"

UserInterface::UserInterface(sf::RenderWindow * aWindow, std::string pName, glm::vec3 pPosition)
	: GameObject(pName, pPosition), _window(aWindow)
{
	assert(_window != NULL);
	UserInterface::_objects = std::vector<UITexture*>();

	UITexture* corkBoard = new UITexture(_window, "corkboard.png");

	NewGameButton* newGame = new NewGameButton(_window, "New_Game.png", "New_Game_selected.png", glm::vec2(100, 40));
	ContinueButton* continueButton = new ContinueButton(_window, "continue.png", "Continue_selected.png", glm::vec2(100, 300));
	LevelSelectButton* levelSelect = new LevelSelectButton(_window, "level_select.png", "level_select_selected.png", glm::vec2(100, 620));
	QuitGameButton* quitGame = new QuitGameButton(_window, "quitpin.png", "quitselected.png", glm::vec2(550, 700));
}

void UserInterface::update(float pStep) {
	if (Paused)
		return;

	sf::Vector2i mousePos = sf::Mouse::getPosition(*_window);
	//check for hovering over objects
	for (int i = _buttons.size() - 1; i > -1; i--) {
		sf::Vector2f objPos = _buttons[i]->GetPosition();
		sf::IntRect objRect = _buttons[i]->GetRect();

		if ((mousePos.x > objPos.x) && (mousePos.x < objPos.x + objRect.width) &&
			(mousePos.y > objPos.y) && (mousePos.y < objPos.y + objRect.height))
		{
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
				_buttons[i]->OnClick();
			}
			_buttons[i]->OnHover();
		}
		else if(_buttons[i]->_hovering) {
			_buttons[i]->OnStopHover();
		}
	}
}


void UserInterface::Add(UITexture* pObject) {
	_objects.push_back(pObject);
}

void UserInterface::AddButton(MenuButton* pObject) {
	_buttons.push_back(pObject);
}

void UserInterface::draw()
{
	for (size_t i = 0; i < _objects.size(); i++) {
		if (i < _objects.size()) {
			_objects[i]->draw();
		}
	}

	for (size_t i = 0; i < _buttons.size(); i++) {
		if (i < _buttons.size()) {
			_buttons[i]->draw();
		}
	}
}

UserInterface::~UserInterface()
{
	//dtor
}
