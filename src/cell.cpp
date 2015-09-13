/*
 * * @author ugra narasimha
 * @date 12 Sep 2015
 */
#include "../includes/cell.hpp"

cCell::cCell()
{
	Capacity = 800*3600;		//800 mAH
	InitialVoltage = 12;		//12 V
	SeriesResistance = 0.001; 	//10 mOhm
	Shift1 = 10;
	Shift2 = 85;
	Drop1 = 5;
	Drop2 = 10;
	AttachedTo = (cBattery*)0;
	Locked = false;
}

bool cCell::setInitialVoltage(double initv)
{
	if(Locked)
		return false;
	InitialVoltage = initv;
	return true;
}

bool cCell::setSeriesResistance(double sres)
{
	if(Locked)
		return false;
	SeriesResistance = sres;
	return true;
}

bool cCell::setCapacity(double cap)
{
	if(Locked)
		return false;
	Capacity = cap * 3600;
	return true;
}

bool cCell::setShiftingPoints(double sh1, double sh2)
{
	if(Locked)
		return false;
	if(sh2 <= sh1)
		return false;
	Shift1 = sh1;
	Shift2 = sh2;
	return true;
}

bool cCell::setDropAmounts(double d1, double d2)
{
	if(Locked)
		return false;
	if(d1>100 ||d1<0 ||d2 >100 ||d2<0)
		return false;
	if(d2 <= d1)
		return false;
	Drop1 = d1;
	Drop2 = d2;
	return true;
}
bool cCell::lock(cBattery* owner)
{
	if(Locked)
		return false;
	AttachedTo = owner;
	Locked = true;
	initialise();
	return true;
}
bool cCell::unlock(cBattery* owner)
{
	if(!Locked)
		return false;
	if(AttachedTo != owner)
		return false;
	AttachedTo = (cBattery*)0;
	Locked = false;
	return true;
}

double cCell::getInitialVoltage(void)
{
	double result;
	AccessSynchroniser.lock();
	result = InitialVoltage;
	AccessSynchroniser.unlock();
	return result;
}

double cCell::getCurrentVoltage(void)
{
	double result;
	AccessSynchroniser.lock();
	result = CurrentVoltage;
	AccessSynchroniser.unlock();
	return result;
}

double cCell::getSeriesResistance(void)
{
	double result;
	AccessSynchroniser.lock();
	result = SeriesResistance;
	AccessSynchroniser.unlock();
	return result;
}

double cCell::getSourceCurrent(void)
{
	double result;
	AccessSynchroniser.lock();
	result = SourceCurrent;
	AccessSynchroniser.unlock();
	return result;
}

double cCell::getCapacity(void)
{
	double result;
	AccessSynchroniser.lock();
	result = Capacity/3600;
	AccessSynchroniser.unlock();
	return result;
}

double cCell::getRemainingCapacityPercentage(void)
{
	return RemainigCapacity;
}

void cCell::initialise(void)
{
	AccessSynchroniser.lock();
	m1 = (InitialVoltage * Drop1) / (Capacity * Shift1);
	m2 = (InitialVoltage * (Drop2 - Drop1)) / (Capacity * (Shift2 - Shift1));
	m3 = (InitialVoltage * (100 - Drop2)) / (Capacity * (100 - Shift2));

	CurrentVoltage = InitialVoltage;
	DischargedCapacity = 0;
	RemainigCapacity = 100;
	Gradient = m1;
	ConstantK = 0;
	AccessSynchroniser.unlock();
	return;
}


bool cCell::update(cBattery* owner,bool connected, double scurrent, double runtime)
{
	if(AttachedTo != owner)
		return false;
	if(0 == runtime)
		return false;
	if(!connected)
	{
		SourceCurrent = 0;
		return true;
	}
	AccessSynchroniser.lock();
	SourceCurrent = scurrent;
	DischargedCapacity += (SourceCurrent * runtime);
	RemainigCapacity = ((Capacity - DischargedCapacity) / Capacity) * 100;
	CurrentVoltage = CurrentVoltage - Gradient*SourceCurrent*runtime + ConstantK;

	if((Gradient == m1) && (DischargedCapacity >= ((Shift1*Capacity)/100)))
	{
		Gradient = m2;
		ConstantK += (m2 - m1)*DischargedCapacity;
	}
	if((Gradient == m2) && (DischargedCapacity >= ((Shift2*Capacity)/100)))
	{
		Gradient = m3;
		ConstantK += (m3 - m2)*DischargedCapacity;
	}
	AccessSynchroniser.unlock();
	return true;
}


bool cCell::loadDefaults(cBattery* owner)
{
	if(owner != AttachedTo)
		return false;
	AccessSynchroniser.lock();
	CurrentVoltage = InitialVoltage;
	RemainigCapacity = 100;
	DischargedCapacity = 0;
	Gradient = m1;
	ConstantK = 0;
	AccessSynchroniser.unlock();
	return true;
}
