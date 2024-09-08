xTaskCreatePinnedToCore(
  TaskNano,                // Task function
  "TaskNano",              // Task name
  4096,                    // Stack size in words
  NULL,                    // Task input parameter
  3,                       // Task priority
  &TaskHandleNano,         // Task handle
  1                        // Core ID
);

xTaskCreatePinnedToCore(
  TaskPID,                 // Task function
  "TaskPID",               // Task name
  4096,                    // Stack size in words
  NULL,                    // Task input parameter
  1,                       // Task priority
  &TaskHandlePID,          // Task handle
  1                        // Core ID
);

xTaskCreatePinnedToCore(
  TaskNavbar,              // Task function
  "TaskNavbar",            // Task name
  4096,                    // Stack size in words
  NULL,                    // Task input parameter
  2,                       // Task priority
  &TaskHandleNavbar,       // Task handle
  1                        // Core ID
);

xTaskCreatePinnedToCore(
  TaskDisplay,             // Task function
  "TaskDisplay",           // Task name
  8192,                    // Stack size in words
  NULL,                    // Task input parameter
  2,                       // Task priority
  &TaskHandleDisplay,      // Task handle
  1                        // Core ID
);

xTaskCreatePinnedToCore(
  TaskTouchScreen,         // Task function
  "TaskTouchScreen",       // Task name
  1024,                    // Stack size in words
  NULL,                    // Task input parameter
  2,                       // Task priority
  &TaskHandleTouchScreen,  // Task handle
  1                        // Core ID
);

xTaskCreatePinnedToCore(
  TaskMain,                // Task function
  "TaskMain",              // Task name
  4096,                    // Stack size in words
  NULL,                    // Task input parameter
  1,                       // Task priority
  &TaskHandleMain,         // Task handle
  1                        // Core ID
);