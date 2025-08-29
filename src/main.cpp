#include "FluidSim.h"
#include "Graphics.h"
#include <SFML/Graphics.hpp>

unsigned char bmpHeader[54];
RGBApix pixMap[IMG_HGHT][IMG_WDTH];
enum PlayMode {RNDR, PLAY, PAUSE, RFSH, SAVE};
	//RNDR is actually making the simulation (max speed and CPU)
	//PLAY is playing what has been rendered (PLAY_FPS is the refresh rate and fps; mid CPU usage)
	//PAUSE is when it is paused (PLAY_FPS is the refresh rate for everything but the main image, which is never changed, almost no CPU)
	//RFSH is when a single frame shift has occurred or the image has to refresh for other reasons while in pause mode (immediately changed back to pause) (mid CPU usage)
	//SAVE is when the images are saving as bitmaps (will change to play when done) (max CPU usage)

PlayMode playMode;
int currentFrame=0;

sf::Clock myClock;
const int PLAY_FPS=15;
sf::RenderWindow window(sf::VideoMode(IMG_WDTH, IMG_HGHT+60), "FluidSim v2.1");
sf::Texture texture;
sf::Sprite sprite;
sf::Font font;

void processEvents();
void dsply();
void bmpRndr();

int main()
{
	if (!texture.create(IMG_WDTH, IMG_HGHT))
		return EXIT_FAILURE;

	sprite.setTexture(texture);

	if (!font.loadFromFile(FONT_PATH))
	{
		cout << "font failed to load" << endl;
		return EXIT_FAILURE;
	}

	cout << "initing..." << endl;

	sf::Time frameTime=sf::seconds(1.0/PLAY_FPS);
	myClock.restart();
	init();
	rndrInit();

	cout << endl << "program started" << endl;

    while (window.isOpen())
    {
    	//if (playMode==RNDR)
			//cout << endl << "cycle " << cycle << endl;

    	processEvents();

    	drawImage.setBits(&image[currentFrame][0][0]);

    	///basic functions

    	if (playMode==RNDR)
		{
			base();
			rndr();
		}
		if (playMode==SAVE)
			bmpRndr();

		//if (playMode==RNDR)
			//cout << "time for this frame was " << myClock.getElapsedTime().asSeconds() << endl;

		//while (myClock.getElapsedTime().asSeconds()<1.0/PLAY_FPS) {}
		if (myClock.getElapsedTime()<frameTime)
			sf::sleep(frameTime-myClock.getElapsedTime());

		myClock.restart();

		dsply();

        switch (playMode)
        {
		case RNDR:
			cycle++;
			currentFrame=cycle;
			if (cycle>=FRAME_NUM)
			{
				playMode=RFSH;
				currentFrame=0;
			}
			break;

		case PLAY:
		case SAVE:
			if ((++currentFrame)>=cycle)
			{
				currentFrame=0;
				playMode=PLAY;
			}
			break;

		case RFSH:
			playMode=PAUSE;

		default:
			break;
        }
    }

	cout << endl << "all done" << endl;

    return 0;
}

void processEvents()
{
	///event polling

	//cout << "event polling" << endl;

	sf::Event event;
	while (window.pollEvent(event))
	{
		switch (event.type)
		{
		// window closed
		case sf::Event::Closed:
			window.close();
			break;

		// key pressed
		case sf::Event::KeyPressed:

			switch (event.key.code)
			{
			case sf::Keyboard::Space:
				if (playMode==PAUSE)
					playMode=PLAY;
				else
				{
					playMode=RFSH;
					if ((--currentFrame)<0)
						currentFrame=0;
				}
				break;

			case sf::Keyboard::Return:
				if (playMode!=RNDR && cycle<FRAME_NUM)
				{
					currentFrame=cycle;
					playMode=RNDR;
				}
				else
				{
					playMode=PLAY;
				}
				break;

			case sf::Keyboard::S:
				currentFrame=0;
				playMode=SAVE;
				break;

			case sf::Keyboard::Right:
				if (playMode==PAUSE)
				{
					playMode=RFSH;

					if ((++currentFrame)>=cycle)
						currentFrame=0;
				}
				break;

			case sf::Keyboard::Left:
				if (playMode==PAUSE)
				{
					playMode=RFSH;

					if ((--currentFrame)<0)
						currentFrame=cycle-1;
				}
				break;

			case sf::Keyboard::Escape:
				window.close();
				playMode=RFSH;
				break;

			default:
				break;
			}

		// we don't process other types of events
		default:
			break;
		}
	}
}

void dsply()
{
	int x, y;

	RGBpix color;


	///text setup

	//status text
	sf::Text statusTxt;
	statusTxt.setFont(font);
	statusTxt.setCharacterSize(24); // in pixels, not points!
	statusTxt.move(10, IMG_HGHT);

	//fps counter (fps not implemented yet)
	/*sf::Text fpsTxt;
	fpsTxt.setFont(font);
	fpsTxt.setCharacterSize(24); // in pixels, not points!
	fpsTxt.move(10, IMG_HGHT);*/


	///progress bar

	//background border
	sf::RectangleShape barBknd(sf::Vector2f(IMG_WDTH*0.75, 20));
	barBknd.setFillColor(sf::Color::Transparent);
	barBknd.setOutlineColor(sf::Color(192, 192, 192, 255));
	barBknd.setOutlineThickness(4);
	barBknd.move(IMG_WDTH*0.25-4, IMG_HGHT+8);

	//rendered rect
	sf::RectangleShape barRndr(sf::Vector2f(grdnt(cycle, 0, FRAME_NUM, 0, IMG_WDTH*0.75-8), 20-8));
	barRndr.setFillColor(sf::Color::Blue);
	barRndr.move(IMG_WDTH*0.25-4+4, IMG_HGHT+8+4);

	//indicator circle
	sf::CircleShape barIndctr(8);
	barIndctr.setFillColor(sf::Color::Black);
	barIndctr.setOutlineColor(sf::Color::Cyan);
	barIndctr.setOutlineThickness(4);
	barIndctr.move(grdnt(currentFrame, 0, FRAME_NUM-1, IMG_WDTH*0.25-4-8+4, IMG_WDTH-4-8-4), IMG_HGHT+8+2);


	///refresh texture

	if (playMode!=PAUSE)
	{
		for (y=0; y<IMG_HGHT; ++y)
		{
			for (x=0; x<IMG_WDTH; ++x)
			{
				color=image[currentFrame][y][x];
				pixMap[IMG_HGHT-y-1][x]=clr(color.r, color.g, color.b, 255);
			}
		}

		texture.update((sf::Uint8*)&pixMap[0][0].r);
	}


	///set status text

	switch (playMode)
	{
	case RNDR:
		statusTxt.setString("Rendering...");
		statusTxt.setFillColor(sf::Color::Blue);
		break;

	case PLAY:
		statusTxt.setString("Play");
		statusTxt.setFillColor(sf::Color::Cyan);
		break;

	case PAUSE:
	case RFSH:
		statusTxt.setString("Pause");
		statusTxt.setFillColor(sf::Color::Cyan);
		break;

	case SAVE:
		statusTxt.setString("Saving...");
		statusTxt.setFillColor(sf::Color::Green);
		break;
	}


	///render to the window

	window.clear();
	window.draw(sprite);
	window.draw(barBknd);
	window.draw(barRndr);
	window.draw(barIndctr);
	window.draw(statusTxt);
	window.display();
}

void bmpRndr()
{
	char filename[256];

	sprintf(filename, "export/image%d", currentFrame);

	drawImage.save(filename);
}

