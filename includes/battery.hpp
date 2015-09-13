
/* * @date 11/09/2015
 * @author Ugra Narasimha
 **/
#ifndef  BATTERY_CLASS
#define  BATTERY_CLASS

#include "cell.hpp"
#include <thread>
#include <mutex>


class cBattery
{
	public:
		cBattery();
		bool reset(void);
		bool run(double load,double resolution,double speed);
		bool stop(void);
		bool addCell(cCell* AdCell);
		double getVout(void);
		double getIout(void);
		bool setCutOffVoltage(double cutoff);
		double getElapsedTime(void);
		bool getSwitchState(int Cell);
		bool IsRunning(void);
	private:
		cCell *Cell[3];		
		bool Switch[3];		
		double Vout;		
		double Iout;		
		double ElapsedTime;	
		double CutOffVoltage;	
		double tollarance;
		std::thread* Runner;	
		std::mutex SimulatorState;	
		void runBattery(double load,double resolution,double speed);
		bool ContinueRunning(void);
		int count;				
		std::mutex AccessSynchroniser; };

#endif //BATTERY_CLASS
