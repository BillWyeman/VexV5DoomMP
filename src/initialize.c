#include "main.h"

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
extern void D_DoomMain(void);


char *client_argvs[] = {
	"bbbb.exe",
	"-connect",
	"1.2.3.4",
	"sfsdf",
	NULL
};
	
char *server_argvs[] = {
	"bbbb.exe",
	"-altdeath",
	"-server",
	"-privateserver",
	NULL
};

char *sp_argvs[] = {
	"bbbb.exe",
	"bbbb",
	"bbbb",
	"bbbb",
	NULL
};

int		myargc = 4;
char**	myargv = server_argvs;

uint8_t DOOM_VEXlink_port = 21;

void initialize() {
	/*
	delay(50);
	link_init_override(20, "DOOM", E_LINK_TRANSMITTER);

	while (!link_connected(20)) {
      delay(20);
    }
    */
    printf("Starting DOOM\n");
    //task_set_priority(CURRENT_TASK, TASK_PRIORITY_MAX);
    D_DoomMain();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
    printf("disabled\n");
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}
