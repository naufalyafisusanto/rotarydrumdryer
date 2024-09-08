float fuzzy_light[] = {600, 600, 900};
float fuzzy_medium[] = {600, 900, 1200};
float fuzzy_heavy[] = {900, 1200, 1500};
float fuzzy_veryheavy[] = {1200, 1500, 1500};

float fuzzy_slow[] = {55, 55, 65};
float fuzzy_normal[] = {55, 65, 75};
float fuzzy_fast[] = {65, 75, 75};

bool fuzzy_state = false;
bool continue_session = false;
bool running_start = true;
bool running_start_setup = true;
bool running_stop = false;
bool running_stop_setup = true;

bool simulation = false;
bool connection = true;
bool running = false;
int8_t rotation = 0;

bool overflow = false;
bool redraw_bg = false;
int8_t state_display = 0; 

unsigned long startMillis = 0;