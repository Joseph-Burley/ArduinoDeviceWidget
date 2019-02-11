# ArduinoDeviceWidget
A node.js server to take sensor data from an Arduino and display it on a webpage.

# Server-Client Communication
The current state of the IoT devices is stored as a JSON file. When the user requests an update through the webpage, the server reads the JSON file(s) and sends the resulting string to the client. The user can view multiple devices on the page at once and request an update on multiple devices in a single request. The page does not dynamically alter the layout of the display (it is currently hard-coded to 5 devices per row), but that is a planned feature.

# Server-Device Communication
The Arduino code run FreeTROS, an open-source real time operating system designed for micro-controllers. The plan for the Arduinos was to run three tasks in FreeRTOS: one task for hardware I/O (switches, sensors, LEDs, etc.), one task for network communication, and one task for serial communication for debugging. Testing found that using an Arduino UNO was unfeasable; it simply does not have enough memory to run both a dedicated serial task and a dedicated ethernet task. An Arduino MEGA, however, does appear to have enough memory. The current plan is to split the Arduino code into an UNO specific version and MEGA specific version. The UNO version will not have the serial task and will communicate only through ethernet.

The application layer communication protocol is still undecided. The original plan was to send a JSON string over a TCP connection from the Arduino to the server. The previous testing, however, suggests that an Arduino won't have sufficient memory to serialize and deserialize JSON strings. The Arduino will probably end up communicating with byte streams.
