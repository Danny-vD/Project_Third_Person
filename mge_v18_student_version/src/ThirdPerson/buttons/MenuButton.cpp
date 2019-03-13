#include <cassert>
#include <iostream>

#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include "ThirdPerson/buttons/MenuButton.hpp"
#include "ThirdPerson/config.hpp"

bool MenuButton::_played = false;

MenuButton::MenuButton(sf::RenderWindow * aWindow, std::string fileName, std::string hoverFileName, glm::vec2 pPosition, std::string pName) : UITexture(aWindow, fileName, pPosition, pName), _selectSound(SoundType::SOUND, config::THIRDPERSON_AUDIO_PATH + "selectsound.flac"), _clickSound(SoundType::SOUND, config::THIRDPERSON_AUDIO_PATH + "penclick.wav")
{
	assert(_window != NULL);

	if (!_texture.loadFromFile(config::THIRDPERSON_TEXTURE_PATH + fileName)) {
		std::cout << "Could not load texture, exiting..." << std::endl;
		return;
	}

	if (!_hoverTexture.loadFromFile(config::THIRDPERSON_TEXTURE_PATH + hoverFileName)) {
		std::cout << "Could not load hover texture, exiting..." << std::endl;
		return;
	}

	_sprite.setTexture(_texture);
	std::cout << _sprite.getTextureRect().height;

}

void MenuButton::OnClick()
{
	_clickSound.Play();
}

void MenuButton::OnHover()
{
	if (!_played)
	{
		_selectSound.Play();
		_played = true;
	}
	_sprite.setTexture(_hoverTexture);
	if (!_hovering) _hovering = true;
}

void MenuButton::OnStopHover()
{
	_sprite.setTexture(_texture);
	_hovering = false;
	_played = false;
}

MenuButton::~MenuButton()
{
	//dtor
}
