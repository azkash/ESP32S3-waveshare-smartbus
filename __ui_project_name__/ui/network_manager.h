#pragma once
#include <Arduino.h>

void network_init();
void network_loop();
void sendClearCallToMaster(const char* seatId);
void sendUnpairSeatToMaster(const char* seatId);
void sendAssignSeatIdToMaster(const char* uid, const char* seatId);
