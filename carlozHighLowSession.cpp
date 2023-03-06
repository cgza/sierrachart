// The top of every source code file must include this line
#include "sierrachart.h"

// For reference, refer to this page:
// https://www.sierrachart.com/index.php?page=doc/AdvancedCustomStudyInterfaceAndLanguage.php

// This line is required. Change the text within the quote
// marks to what you want to name your group of custom studies. 
SCDLLName("carlozHighLowSession")

SCSFExport scsf_HighLowSession(SCStudyInterfaceRef sc)
{
	SCSubgraphRef Subgraph_IBHigh   = sc.Subgraph[0];
	SCSubgraphRef Subgraph_IBMid    = sc.Subgraph[1];
	SCSubgraphRef Subgraph_IBLow    = sc.Subgraph[2];

	
	SCInputRef Input_Session      = sc.Input[0];
	SCInputRef Input_NumberDaysToCalculate = sc.Input[1];


	if (sc.SetDefaults)
	{
		sc.GraphName		= "Session's High/Low";
		sc.DrawZeros		= 0;
		sc.GraphRegion		= 0;
		sc.AutoLoop			= 1;

		sc.ScaleRangeType = SCALE_SAMEASREGION;


		Subgraph_IBHigh.Name = "High";
		Subgraph_IBHigh.PrimaryColor = RGB(128, 255, 128);
		Subgraph_IBHigh.DrawStyle = DRAWSTYLE_DASH;
		Subgraph_IBHigh.DrawZeros = false;

		Subgraph_IBMid.Name = "Mid";
		Subgraph_IBMid.PrimaryColor = RGB(255, 255, 255);
		Subgraph_IBMid.DrawStyle = DRAWSTYLE_DASH;
		Subgraph_IBMid.DrawZeros = false;

		Subgraph_IBLow.Name = "Low";
		Subgraph_IBLow.PrimaryColor = RGB(255, 128, 128);
		Subgraph_IBLow.DrawStyle = DRAWSTYLE_DASH;
		Subgraph_IBLow.DrawZeros = false;


		// Inputs
		Input_Session.Name = "High/Low for Session";
		Input_Session.SetCustomInputStrings("Day Session (RTH); Evening Session (ETH)");
		Input_Session.SetCustomInputIndex(0);

		Input_NumberDaysToCalculate.Name = "Number of Days to Calculate";
		Input_NumberDaysToCalculate.SetInt(100);
		Input_NumberDaysToCalculate.SetIntLimits(1,INT_MAX);

		return;
	}

	// Persist vars
	int& PeriodFirstIndex = sc.GetPersistentInt(1);
	
	SCDateTime& PeriodStartDateTime = sc.GetPersistentSCDateTime(1);
	SCDateTime& PeriodEndDateTime   = sc.GetPersistentSCDateTime(2);

	float& PeriodHigh       = sc.GetPersistentFloat(1);
	float& PeriodLow        = sc.GetPersistentFloat(2);
	float& PeriodMid        = sc.GetPersistentFloat(3);


	// Reset persistent variables upon full calculation
	if (sc.Index == 0)
	{
		PeriodFirstIndex = -1;
		PeriodStartDateTime = 0;
		PeriodEndDateTime   = 0;
		PeriodHigh = -FLT_MAX;
		PeriodLow  =  FLT_MAX;
	}

	SCDateTimeMS LastBarDateTime = sc.BaseDateTimeIn[sc.ArraySize-1];
	SCDateTimeMS FirstCalculationDate = LastBarDateTime.GetDate() - SCDateTime::DAYS(Input_NumberDaysToCalculate.GetInt() - 1);

	SCDateTimeMS CurrentBarDateTime = sc.BaseDateTimeIn[sc.Index];

	SCDateTimeMS PrevBarDateTime;

	if (sc.Index > 0)
		PrevBarDateTime = sc.BaseDateTimeIn[sc.Index-1];

	if (CurrentBarDateTime.GetDate() < FirstCalculationDate) // Limit calculation to specified number of days back
		return;

	SCDateTimeMS StartDateTime = CurrentBarDateTime;
	
	//Set StartTime for selected session
	if (Input_Session.GetIndex() == 0)
		StartDateTime.SetTime(sc.StartTime1);
	else
		StartDateTime.SetTime(sc.StartTime2);
		
	if (PrevBarDateTime < StartDateTime && CurrentBarDateTime >= StartDateTime)
	{
		PeriodFirstIndex = sc.Index;
		PeriodHigh = -FLT_MAX;
		PeriodLow  = FLT_MAX;

		PeriodStartDateTime = StartDateTime;
		
		//Set Period End Time as end of selected session
		PeriodEndDateTime = PeriodStartDateTime;
		if (Input_Session.GetIndex() == 0)
			PeriodEndDateTime.SetTime(sc.EndTime1 - 1);
		else
			PeriodEndDateTime.SetTime(sc.EndTime2 - 1);
		
		//add a day if EndTime is on the next day
		if(PeriodEndDateTime < PeriodStartDateTime)
			PeriodEndDateTime.AddDays(1);
	}

	// Check end of period
	if (PeriodFirstIndex >= 0)
	{
		if (CurrentBarDateTime > PeriodEndDateTime)
		{
			PeriodFirstIndex = -1;
		}
	}

	// Collecting data, back propagate if changed
	if (PeriodFirstIndex >= 0)
	{
		bool Changed = false;

		if (sc.High[sc.Index] > PeriodHigh)
		{
			PeriodHigh = sc.High[sc.Index];
			Changed = true;
		}

		if (sc.Low[sc.Index] < PeriodLow)
		{
			PeriodLow = sc.Low[sc.Index];
			Changed = true;
		}

		if (Changed)
		{
			PeriodMid = (PeriodHigh + PeriodLow) / 2.0f;

			for (int Index = PeriodFirstIndex; Index < sc.Index; Index++)
			{
				Subgraph_IBHigh[Index]  = PeriodHigh;
				Subgraph_IBLow[Index]   = PeriodLow;
				Subgraph_IBMid[Index]   = PeriodMid;			
			}

			sc.EarliestUpdateSubgraphDataArrayIndex = PeriodFirstIndex;
		}
	}

	// Plot current values
	if (PeriodLow != FLT_MAX)
	{
		Subgraph_IBHigh[sc.Index]  = PeriodHigh;
		Subgraph_IBLow[sc.Index]   = PeriodLow;
		Subgraph_IBMid[sc.Index]   = PeriodMid;
	}
}