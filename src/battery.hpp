/*
 * @author ugra narasimha
 * @date 12 Sep 2015
 */

#include "../includes/battery.hpp"
#include <unistd.h>
#include <iostream> 

cBattery::cBattery()
{
	count = 0;
	Vout = 0;
	Iout = 0;
	ElapsedTime = 0;
	CutOffVoltage = 7;
	tollarance = 0.005; //50mV
	SimulatorState.unlock();
	for(int i=0; i<3; i++)
		Switch[i] = false;
}

bool cBattery::getSwitchState(int cell)
{
	bool result;
	AccessSynchroniser.lock();
	result = Switch[cell];
	AccessSynchroniser.unlock();
	return result;
}

double cBattery::getElapsedTime(void)
{
	double result;
	AccessSynchroniser.lock();
	result = ElapsedTime;
	AccessSynchroniser.unlock();
	return result;
}

bool cBattery::reset(void)
{
	bool status = false;
	bool lockStatus = false;
	for(int i=0; i<count; i++)
	{
		lockStatus = Cell[i]->lock(this);
		status = Cell[i]->loadDefaults(this);
		if(!status)
			break;
		if(lockStatus)
			Cell[i]->unlock(this);
	}
	ElapsedTime = 0;
	Vout = 0;
	Iout = 0;
	return status;
}

bool cBattery::run(double load,double resolution,double speed)
{
	if(IsRunning())
		return false;
	SimulatorState.lock();
	Runner = new std::thread(&cBattery::runBattery, this, load, resolution, speed);
	return true;
}

bool cBattery::stop(void)
{
	if(IsRunning())
	{
		SimulatorState.unlock();
		Runner->join();
		return true;
	}
	return false;
}

bool cBattery::addCell(cCell* AdCell)
{
	if(IsRunning())
		return false;
	if(count>=3)
		return false;
	Cell[count++] = AdCell;
	return true;
}

bool cBattery::setCutOffVoltage(double cutoff)
{
	if(IsRunning())
		return false;
	AccessSynchroniser.lock();
	CutOffVoltage = cutoff;
	AccessSynchroniser.unlock();
	return true;
}


double cBattery::getVout(void)
{
	double result;
	AccessSynchroniser.lock();
	result = Vout;
	AccessSynchroniser.unlock();
	return result;
}

double cBattery::getIout(void)
{
	double result;
	AccessSynchroniser.lock();
	result = Iout;
	AccessSynchroniser.unlock();
	return (result*1000);
}


bool cBattery::IsRunning(void)
{
	return ContinueRunning();
}


bool cBattery::ContinueRunning(void)
{
	if(SimulatorState.try_lock())
	{
		SimulatorState.unlock();
		return false;
	}
	return true;
}


void cBattery::runBattery(double load, double resolution, double speed)
{
	if(resolution == 0 || speed == 0 || load == 0)
		return;
	bool status = false;
	int i;
	double seriesResistance[count];
	for(i=0; i<count; i++)
	{
		status = Cell[i]->lock(this);
		if(!status)
			return;
		seriesResistance[i] = Cell[i]->getSeriesResistance();
	}

	int j, iTemp;
	double dTemp;
	double outVolt=0;
	bool localSwitch[count];
	int sortedCells[count];
	double cellVoltages[count];
	double tempVoltages[count];
	double sourceCurrent[count];
	double ratio;

	FILE* logFile;
	logFile = fopen("./batsim.log","a");

	fprintf(logFile,"***************************************************\n");
	fprintf(logFile,"\t\t\tBattery Simulator\n");
	fprintf(logFile,"***************************************************\n");
	fprintf(logFile,"\n[%9.3f]\tSimulator Started\n",ElapsedTime/1000);

	while(ContinueRunning())
	{

		fprintf(logFile,"\n[%9.3f]\toutVolt: %f\tIout: %f\n\tCell 1:: %d: %f V,\t%f mA\n\tCell 2:: %d: %f V,\t%f mA\n\tCell 3:: %d: %f V,\t%f mA\n",
			ElapsedTime/1000,Vout,Iout*1000,Switch[0],Cell[0]->getCurrentVoltage(),Cell[0]->getSourceCurrent()*1000,Switch[1],Cell[1]->getCurrentVoltage(),Cell[1]->getSourceCurrent()*1000,Switch[2],Cell[2]->getCurrentVoltage(),Cell[2]->getSourceCurrent()*1000);

	for(i=0;i<count;i++)
	{
		localSwitch[i] = false;
		sortedCells[i] = i;
		cellVoltages[i] = Cell[i]->getCurrentVoltage();
		tempVoltages[i] = cellVoltages[i];
	}

	for(i=0;i<count;i++)
	{
		for(j=i+1;j<count;j++)
		{
			if(tempVoltages[i]<tempVoltages[j])
			{
				iTemp=sortedCells[i];
				dTemp=tempVoltages[i];
				sortedCells[i]=sortedCells[j];
				tempVoltages[i]=tempVoltages[j];
				sortedCells[j]=iTemp;
				tempVoltages[j]=dTemp;
			}
		}
	}

	localSwitch[sortedCells[0]] = true;
	outVolt = cellVoltages[sortedCells[0]];
	for(i=1;i<count;i++)
	{
		if((cellVoltages[sortedCells[0]] - cellVoltages[sortedCells[i]])<=tollarance)
		{
			localSwitch[sortedCells[i]] = true;
			outVolt = cellVoltages[sortedCells[i]];
		}
	}
	ratio = 0;
	for(i=0;i<count;i++)
	{
		if(localSwitch[i])
			ratio += cellVoltages[i]/seriesResistance[i];
	}

	AccessSynchroniser.lock();
	Vout = outVolt;
	Iout = Vout / load;

	for(i=0;i<count;i++)
	{
		if(localSwitch[i])
			sourceCurrent[i] = (Iout*cellVoltages[i])/(ratio * seriesResistance[i]);
		else
			sourceCurrent[i] = 0;
		Switch[i] = localSwitch[i];
		Cell[i]->update(this,localSwitch[i],sourceCurrent[i],resolution);
	}
	AccessSynchroniser.unlock();

		//sleep for Inteval
		usleep(resolution*1000/speed);
		AccessSynchroniser.lock();
		ElapsedTime += resolution;
		AccessSynchroniser.unlock();

		//if total voltage < MIN, break;
		if(outVolt < CutOffVoltage)
		{
			for(i = 0; i<count; i++)
				localSwitch[i] = false;
			SimulatorState.unlock();
			std::cout<<"\nBattery exhausted\nSimulation completed\n";
			fprintf(logFile,"ALERT :: exhausted\n");
			std::cout<<"BatSim >> ";
		}
	}
	fprintf(logFile,"\n[%9.3f]\tSimulator Stopped\n",ElapsedTime/1000);
	fcloseall();
	for(i =0;i<count;i++)
		Cell[0]->unlock(this);
	return;
}
