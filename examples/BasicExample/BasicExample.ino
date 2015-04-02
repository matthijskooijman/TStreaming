// This is a very basic example that shows how to use the library

#include <TStreaming.h>

void setup() 
{
    int x = 10;

    Serial.begin(9600);
    Serial << "Value: " << x << " (0x" << V<Hex>(x) << ")";
}

void loop()
{
    ;
    // nothing here
}
