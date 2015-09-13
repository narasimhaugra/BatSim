/*
 * @author ugra narasimha
 * @date 12 Sep 2015
 */
#include "../includes/simulator.hpp"
#include <iostream>


cBatSim::cBatSim(void)
{
	Resolution = 100;
	Speed = 1;
	BatteryConnected = false;
}


cBatSim::cBatSim(int multiplier, double res)
{
	if(0 == res)
		Resolution = 100;
	else
		Resolution = res;
	if(0 == multiplier)
		Speed = 1;
	else
		Speed = multiplier;
	BatteryConnected = false;
}


bool cBatSim::start(void)
{
	if(!BatteryConnected)
		return false;
	if(BatPack->IsRunning())
		return false;
	std::cout<<"calling battery run"<<std::endl;
	return (BatPack->run(Load,Resolution,Speed));
}


bool cBatSim::stop(void)
{
	if(!BatteryConnected)
		return false;
	if(!BatPack->IsRunning())
		return false;
	std::cout<<"calling battery stop"<<std::endl;
	if(BatPack->stop())
	{
		std::cout<<"battery stopped"<<std::endl;
		return (BatPack->reset());
	}
	return false;
}

bool cBatSim::pause(void)
{
	if(!BatteryConnected)
		return false;
	if(!BatPack->IsRunning())
		return false;
	return (BatPack->stop());
}

bool cBatSim::resume(void)
{
	return start();
}


bool cBatSim::setSpeed(int multiplier)
{
	if(BatteryConnected)
	{
		if(BatPack->IsRunning())
			return false;
	}
	if(0 >= multiplier)
		return false;
	Speed = multiplier;
	return true;
}

bool cBatSim::setResolution(double milisec)
{
	if(BatteryConnected)
	{
		if(BatPack->IsRunning())
			return false;
	}
	if(0 >= milisec)
		return false;
	Resolution = milisec;
	return true;
}

bool cBatSim::connect(cBattery* battery)
{
	if(BatteryConnected)
	{
		if(BatPack->IsRunning())
			return false;
	}
	BatPack = battery;
	BatteryConnected = true;
	return true;
}

bool cBatSim::connect(double load)
{
	if(BatteryConnected)
	{
		if(BatPack->IsRunning())
			return false;
	}
	Load = load;
	return true;
}

double cBatSim::getLoad(void)
{
	return Load;
}
