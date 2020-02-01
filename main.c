#define START_BUTTON 1
#define A_BUTTON 2
#define B_BUTTON 3
#define C_BUTTON 4

#define PLAYER_1 1
#define PLAYER_2 2

#include <genesis.h>
#include <resources.h>

//Screen
int desiredScreenWidth = 320;
int desiredScreenHeight = 224;

//Background MAps
Map *mapBackground;
Map *cloudBackground;

//Player
struct Player{
	Sprite *playerSprite;
	fix16 posX;
	fix16 velX;
	fix16 posY;
	fix16 velY;
	int horizontalNormal;
	int verticalNormal;
	bool jumping;
	bool isMoving;
	int moveConstraintXLeft;
	int moveConstraintXRight;
};

struct Player player1;
struct Player player2;
struct Player players[2];

const int playerWidth = 64;
const int playerHeight = 64;
const int jumpForce = -3;
const fix16 jumpDistance = FIX16(0.75);

const int groundHeight = 180;


int scrollSpeed = 1;
int scrollAmount;
int frameCount = 0;

void init();
void setupPlayField();
void setupPlayers();
void gravity();
void playerJumping();
void playerWalking();
void setPlayerPosition();

fix16 SineEaseInOut(fix16 p);
int countFrames();
void ScrollBackground();
void setupMusic();
//Button Functions
int p1ButtonPressEvent(int button);
int p2ButtonPressEvent(int button);

static void
myJoyHandler(u16 joy, u16 changed, u16 state);
int main()
{
	init();
	while (1)
	{
		countFrames();
		ScrollBackground();
		//Updates Sprites Position / Animation Frame
		SPR_update();
		gravity();
		setPlayerPosition();
		//Wait for the frame to finish rendering
		VDP_waitVSync();
	}
	return(0);
	
}

void init()
{
	//Input
	JOY_init();
	JOY_setEventHandler(&myJoyHandler);

	//Set up the resolution
	VDP_setScreenWidth320();
	VDP_setPlanSize(64, 32);
	
	//Set the background color
	//Manually sets a pallete colour to a hex code
	//First pallet is the background color
	//SHANE THIS IS USEFUL
	VDP_setPaletteColor(0, RGB24_TO_VDPCOLOR(0x6dc2ca));

	SPR_init(0, 0, 0);
	setupPlayField();
	setupPlayers();
	setupMusic();
}

void setupMusic()
{
	XGM_setLoopNumber(10);
	//Load PCM 65 with the start sound effect, how long it is
	//XGM_setPCM(65, &start, 17664);
	//Play PCM 65, Priority 1 signal from starflet, sound channel 2
	//XGM_startPlayPCM(65, 1, SOUND_PCM_CH2);
	//Play our Music
	XGM_startPlay(&music);

}

void setupPlayField()
{
	//Set up the map tilesets
	VDP_setPalette(PAL3, BGBuildings.palette->data);
	VDP_loadTileSet(BGBuildings.tileset, 1, DMA);
	VDP_setScrollingMode(HSCROLL_PLANE,VSCROLL_PLANE);
	mapBackground = unpackMap(BGBuildings.map, NULL);

	//Background art is using Palette 3
	VDP_setMapEx(PLAN_A, mapBackground, TILE_ATTR_FULL(PAL3, 0, FALSE, FALSE, 1), 0, 0, 0, 0, 64, 28);

	VDP_loadTileSet(BGClouds.tileset, 320, DMA);
	cloudBackground = unpackMap(BGClouds.map, NULL);
	VDP_setMapEx(PLAN_B, cloudBackground, TILE_ATTR_FULL(PAL3, 0, FALSE, FALSE, 320), 0, 0, 0, 0, 64, 28);
	//Set the background color
	//Manually sets a pallete colour to a hex code
	//First pallet is the background color
	//SHANE THIS IS USEFUL
	VDP_setPaletteColor(0, RGB24_TO_VDPCOLOR(0x00000));
}

void setupPlayers()
{ 
	//Sprites are using Palette 1
	VDP_setPalette(PAL1, player1Sprite.palette->data);

	players[0] = player1;
	players[1] = player2;

	//Set the players intial position
	player1.posX = intToFix16(0);
	player1.posY = intToFix16(64 - playerHeight);
	player1.moveConstraintXLeft = 0;
	player1.moveConstraintXRight = (screenWidth / 2) - playerWidth;
	player2.posX = intToFix16(256);
	player2.posY = intToFix16(64 - playerHeight);
	player2.moveConstraintXLeft = screenWidth / 2;
	player2.moveConstraintXRight = 256;

	//Insert the player sprites at the above positions
	player1.playerSprite = SPR_addSprite(&player1Sprite, fix16ToInt(player1.posX), fix16ToInt(player1.posY), TILE_ATTR(PAL1, 0, FALSE, FALSE));
	player2.playerSprite = SPR_addSprite(&player1Sprite, fix16ToInt(player2.posX), fix16ToInt(player2.posY), TILE_ATTR(PAL1, 0, FALSE, TRUE));
	SPR_update();
}

int countFrames()
{
	frameCount++;
	if(frameCount>60)
	{
		frameCount = 0;
	}
	return frameCount;
}

void gravity()
{
	//Apply Velocity, need to use fix16Add to add two "floats" together
	player1.posY = fix16Add(player1.posY, player1.velY);
	player1.posX = fix16Add(player1.posX, player1.velX);

	player2.posY = fix16Add(player2.posY, player2.velY);
	player2.posX = fix16Add(player2.posX, player2.velX);

	//Check if player is on floor
	if (fix16ToInt(player1.posY) + playerHeight >= groundHeight)
	{
		player1.jumping = FALSE;
		player1.velY = FIX16(0);
		player1.posY = intToFix16(groundHeight - playerHeight);
		player1.velX = intToFix16(0);
	}
	else
	{
		player1.velY = fix16Add(player1.velY, 6);
	}

	if (fix16ToInt(player2.posY) + playerHeight >= groundHeight)
	{
		player2.jumping = FALSE;
		player2.velY = FIX16(0);
		player2.posY = intToFix16(groundHeight - playerHeight);
		player2.velX = intToFix16(0);
	}
	else
	{
		player2.velY = fix16Add(player2.velY, 6);
	}
}

void playerJumping(int player, int direction)
{
	int jumpDirection = direction;
	if(player == PLAYER_1 && player1.jumping != TRUE)
	{
		player1.jumping = TRUE;
		player1.velY = FIX16(jumpForce);
		player1.velX = fix16Mul(intToFix16(direction), jumpDistance);
	}

	if (player == PLAYER_2 && player1.jumping != TRUE)
	{
		player2.jumping = TRUE;
		player2.velY = FIX16(jumpForce);
		player2.velX = fix16Mul(intToFix16(direction), jumpDistance);
	}
}

void playerWalking()
{
	
	if(player1.horizontalNormal == 1)
	{
		if(player1.posX < intToFix16(player1.moveConstraintXRight))
		{
			player1.posX += intToFix16(playerWidth / 2);
		}
	}
	else if(player1.horizontalNormal == -1)
	{
		if(player1.posX > intToFix16(player1.moveConstraintXLeft))
		{
			player1.posX -= intToFix16(playerWidth / 2);
		}
	}

	if(player2.horizontalNormal == 1)
	{
		if(player2.posX < intToFix16(player2.moveConstraintXRight))
		{
			player2.posX += intToFix16(playerWidth / 2);
		}
	}
	else if(player2.horizontalNormal == -1)
	{
		if(player2.posX > intToFix16(player2.moveConstraintXLeft))
		{
			player2.posX -= intToFix16(playerWidth / 2);
		}
	}
}

void setPlayerPosition()
{
	SPR_setPosition(player1.playerSprite, fix16ToInt(player1.posX), fix16ToInt(player1.posY));
	SPR_setPosition(player2.playerSprite, fix16ToInt(player2.posX), fix16ToInt(player2.posY));
}

//Easings
fix16 SineEaseInOut(fix16 p)
{
	return FIX16(0.5 * (1 - cosFix16( p * 3 )));
}

//Background Effects
void ScrollBackground()
{
	if(frameCount % 6 == 0)
	{
		
	VDP_setHorizontalScroll(PLAN_B, scrollAmount -= scrollSpeed);
		
	}
}

//Input Stuff
int p1ButtonPressEvent(int button)
{
	if (button == A_BUTTON)
	{
		playerJumping(PLAYER_1, player1.horizontalNormal * 5);
	}
	else
	{

	}
	return (0);
}

int p2ButtonPressEvent(int button)
{
	if (button == A_BUTTON)
	{
		playerJumping(PLAYER_2, player2.horizontalNormal);
	}
	return (0);
}

static void myJoyHandler(u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
		/*Start game if START is pressed*/
		if (state & BUTTON_START)
		{
			p1ButtonPressEvent(START_BUTTON);
		}

		//State = This will be 1 if the button is currently pressed and 0 if it isn’t.
		if (state & BUTTON_A)
		{
			p1ButtonPressEvent(A_BUTTON);
		}else if (state & BUTTON_B)
		{
			p1ButtonPressEvent(B_BUTTON);
		}else if (state & BUTTON_C)
		{
			p1ButtonPressEvent(C_BUTTON);
		}

		if (state & BUTTON_RIGHT)
		{
			player1.horizontalNormal = 1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_RIGHT)
			{
				player1.horizontalNormal = 0;
			}
		}

		if (state & BUTTON_LEFT)
		{
			player1.horizontalNormal = -1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_LEFT)
			{
				player1.horizontalNormal = 0;
			}
		}

		if (state & BUTTON_UP)
		{
			player1.verticalNormal = 1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_UP)
			{
				player1.verticalNormal = 0;
			}
		}

		if (state & BUTTON_DOWN)
		{
			player1.verticalNormal = -1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_DOWN)
			{
				player1.verticalNormal = 0;
			}
		}
	}

	if (joy == JOY_2)
	{
		/*Start game if START is pressed*/
		if (state & BUTTON_START)
		{
			p2ButtonPressEvent(START_BUTTON);
		}

		//State = This will be 1 if the button is currently pressed and 0 if it isn’t.
		if (state & BUTTON_A)
		{
			p2ButtonPressEvent(A_BUTTON);
		}

		if (state & BUTTON_B)
		{
			p2ButtonPressEvent(B_BUTTON);
		}

		if (state & BUTTON_C)
		{
			p2ButtonPressEvent(C_BUTTON);
		}

		if (state & BUTTON_RIGHT)
		{
			player2.horizontalNormal = 1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_RIGHT)
			{
				player2.horizontalNormal = 0;
			}
		}

		if (state & BUTTON_LEFT)
		{
			player2.horizontalNormal = -1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_LEFT)
			{
				player2.horizontalNormal = 0;
			}
		}

		if (state & BUTTON_UP)
		{
			player2.verticalNormal = 1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_UP)
			{
				player2.verticalNormal = 0;
			}
		}

		if (state & BUTTON_DOWN)
		{
			player2.verticalNormal = -1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_DOWN)
			{
				player2.verticalNormal = 0;
			}
		}
	}
}
