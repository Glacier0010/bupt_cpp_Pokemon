#include "hub.h"
#include <ctime>
#include <stdlib.h>

int main()
{
	srand(time(NULL));
	Hub& hub = Hub::getInstance();
	hub.init();
	return 0;
}