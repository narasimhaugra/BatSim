/*
 * @date 11/09/2015
 * @author Ugra Narasimha
 **/
#ifndef  CELL_CLASS
#define  CELL_CLASS

class cBattery;
#include <mutex>


class cCell
{
	public:
		cCell();
		bool setInitialVoltage(double initv);
		bool setSeriesResistance(double sres);
		bool setCapacity(double cap);
		bool setShiftingPoints(double d1, double d2);
		bool setDropAmounts(double sh1, double sh2);
		double getInitialVoltage(void);
		double getSeriesResistance(void);
		double getSourceCurrent(void);
		double getCurrentVoltage(void);
		double getCapacity(void);
		double getRemainingCapacityPercentage(void);
		bool lock(cBattery* owner);
		bool unlock(cBattery* owner);
		bool update(cBattery* owner,bool connected, double scurrent, double runtime);
		bool loadDefaults(cBattery* owner);
	private:
		bool Locked;				
		cBattery* AttachedTo;		
		double InitialVoltage;		
		double SeriesResistance;	
		double Capacity;		
		double Shift1;			
		double Shift2;			
		double Drop1;			
		double Drop2;			
		double Gradient;		
		double ConstantK;		
		double m1,m2,m3;		
		double DischargedCapacity;  
		double RemainigCapacity;	
		double SourceCurrent;		
		double CurrentVoltage;		
		void initialise(void);
		std::mutex AccessSynchroniser;
};

#endif //CELL_CLASS
