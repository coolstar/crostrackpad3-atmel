#include "stdint.h"

typedef enum {
	ThreeFingerTapActionCortana,
	ThreeFingerTapActionWheelClick,
	ThreeFingerTapActionNone
} ThreeFingerTapAction;

typedef enum {
	SwipeUpGestureTaskView,
	SwipeUpGestureNone
} SwipeUpGesture;

typedef enum {
	SwipeDownGestureShowDesktop,
	SwipeDownGestureNone
} SwipeDownGesture;

typedef enum {
	SwipeGestureAltTabSwitcher,
	SwipeGestureSwitchWorkspace,
	SwipeGestureNone
} SwipeGesture;

struct csgesture_settings {
	int pointerMultiplier; //done

	//click settings
	bool swapLeftRightFingers; //done
	bool clickWithNoFingers; //done
	bool multiFingerClick; //done
	bool rightClickBottomRight;

	//tap settings
	bool tapToClickEnabled; //done
	bool multiFingerTap; //done
	bool tapDragEnabled; //done

	ThreeFingerTapAction threeFingerTapAction; //done

	bool fourFingerTapEnabled; //done

	//scroll settings
	int scrollEnabled; //done

	//three finger gestures
	SwipeUpGesture threeFingerSwipeUpGesture;
	SwipeDownGesture threeFingerSwipeDownGesture;
	SwipeGesture threeFingerSwipeLeftRightGesture;

	//four finger gestures
	SwipeUpGesture fourFingerSwipeUpGesture;
	SwipeDownGesture fourFingerSwipeDownGesture;
	SwipeGesture fourFingerSwipeLeftRightGesture;
};

struct csgesture_softc {
	struct csgesture_settings settings;

	//hardware input
	int x[20];
	int y[20];
	int p[20];

	bool buttondown;

	//hardware info
	bool infoSetup;

	char product_id[16];
	char firmware_version[16];

	int resx;
	int resy;
	int phyx;
	int phyy;

	//system output
	int dx;
	int dy;

	int scrollx;
	int scrolly;

	int buttonmask;

	//used internally in driver
	int panningActive;
	int idForPanning;

	int scrollingActive;
	int idsForScrolling[2];
	int ticksSinceScrolling;

	int scrollInertiaActive;

	int blacklistedids[20];

	bool mouseDownDueToTap;
	int idForMouseDown;
	bool mousedown;
	int mousebutton;

	int lastx[20];
	int lasty[20];
	int lastp[20];

	int xhistory[20][10];
	int yhistory[20][10];

	int flextotalx[20];
	int flextotaly[20];

	int totalx[20];
	int totaly[20];
	int totalp[20];

	int multitaskingx;
	int multitaskingy;
	int multitaskinggesturetick;
	bool multitaskingdone;

	bool alttabswitchershowing;

	int idsforalttab[3];

	int tick[20];
	int truetick[20];
	int ticksincelastrelease;
	int tickssinceclick;
};