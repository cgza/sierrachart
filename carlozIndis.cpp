// The top of every source code file must include this line
#include "sierrachart.h"

// For reference, refer to this page:
// https://www.sierrachart.com/index.php?page=doc/AdvancedCustomStudyInterfaceAndLanguage.php

// This line is required. Change the text within the quote
// marks to what you want to name your group of custom studies. 
SCDLLName("carlozIndis")

SCSFExport scsf_AutoVbP(SCStudyInterfaceRef sc)
{
	SCInputRef useVap = sc.Input[0];
    // multiplier that will determine the granularity of VP bars we'll see as we switch symbols
    // the lower the number, the thicker and fewer bars there will be
    // the higher the magic number, the thinner and more VP bars there will be
    SCInputRef i_DetailLevel = sc.Input[1];
	SCInputRef i_Step = sc.Input[2];
	
	//number of inputs to set target VBP studies
	const int MAX_VBP_STUDIES = 20;
	
    // Configuration
    if (sc.SetDefaults)
    {
        sc.GraphRegion = 0;
		sc.GraphName = "Auto set VbPs ticks per bar";
		sc.AutoLoop = 0; 
		//update always so we don't have to wait for incoming ticks (market closed)
		sc.UpdateAlways = 1;
		
		//INPUTS CONFIG
		useVap.Name = "Use VAP setting in Chart Settings";
        useVap.SetYesNo(0);
        i_DetailLevel.Name = "VbPs' detail level";
        i_DetailLevel.SetInt(100);
		i_Step.Name = "Tick-size step to increase/decrease detail";
		i_Step.SetInt(4);
		
		for(int x = 0; x < MAX_VBP_STUDIES; x++)
		{
			sc.Input[10 + x].Name.Format("%i.Target VbP study", x + 1);
			sc.Input[10 + x].SetStudyID(0);
		}
		
        return;
    }
	
	if(sc.IsFullRecalculation)
	{
		//set step to the nearest multiplier of sc.VolumeAtPriceMultiplier to avoid problems
		int step = i_Step.GetInt();
		i_Step.SetInt(step - step % sc.VolumeAtPriceMultiplier);
	}
	
	// VbP Ticks Per Volume Bar is input 32, ID 31
    int inputIdx = 31;

	//if setting VbP's Ticks-per-bar according to Volume At Price Multiplier in ChartSettings
    if (useVap.GetInt() == 1) {
		
		for(int x = 0; x < MAX_VBP_STUDIES; x++)
		{
			int studyId = sc.Input[10 + x].GetStudyID();
			if(studyId != 0)
			{
				int prevValue;
				sc.GetChartStudyInputInt(sc.ChartNumber, studyId, inputIdx, prevValue); 			
				if(prevValue != sc.VolumeAtPriceMultiplier)
					sc.SetChartStudyInputInt(sc.ChartNumber, studyId, inputIdx, sc.VolumeAtPriceMultiplier);
			}
		}
		
        return;
    }
	
	//if using detail level specified in inputs
    float vHigh, vLow, vDiff;
    int detail = i_DetailLevel.GetInt();

    // fetch the graph's price scale's high and low value so we can automate the Ticks setting on VbP
    sc.GetMainGraphVisibleHighAndLow(vHigh, vLow);

    // calc the range of visible prices
	//DIVIDE BY TICK-SIZE TO GET NUMBER OF VISIBLE TICKS
    vDiff = (vHigh - vLow) / sc.TickSize;

    // divide range by detail level to get the desired VbP Ticks Per Bar value and don't allow it to be less than the VAP multiplier
    int targetTicksPerBar = max(sc.Round(vDiff / detail), sc.VolumeAtPriceMultiplier);
	
	// adapt targetTicksPerBar to the specified step
	if(targetTicksPerBar >= i_Step.GetInt() )
		targetTicksPerBar -= targetTicksPerBar % i_Step.GetInt();
	//if targetTicksPerBar is less than the step but greater than VAP multiplier, then modify it as a VAP multiplier multiple
	else if(targetTicksPerBar > sc.VolumeAtPriceMultiplier)
		targetTicksPerBar -= targetTicksPerBar % sc.VolumeAtPriceMultiplier;
	
	//flag to redraw chart in case we change any target-VbP
	bool redraw = false;
	
	for(int x = 0; x < MAX_VBP_STUDIES; x++)
	{
		int studyId = sc.Input[10 + x].GetStudyID();
		if(studyId != 0)
		{
			int prevValue;
			sc.GetChartStudyInputInt(sc.ChartNumber, studyId, inputIdx, prevValue);

			if(targetTicksPerBar != prevValue) 
			{
				sc.SetChartStudyInputInt(sc.ChartNumber, studyId, inputIdx, targetTicksPerBar);
				redraw = true;
			}
		}
	}
	
	if(redraw) sc.RecalculateChart(sc.ChartNumber);
}

SCSFExport scsf_PullingStackingForBars(SCStudyInterfaceRef sc)
{
	SCSubgraphRef PSbidUp1 = sc.Subgraph[0];
	SCSubgraphRef PSbidUp2 = sc.Subgraph[1];
	SCSubgraphRef PSbidUp3 = sc.Subgraph[2];
	SCSubgraphRef PSbidDown1 = sc.Subgraph[3];
	SCSubgraphRef PSbidDown2 = sc.Subgraph[4];
	SCSubgraphRef PSbidDown3 = sc.Subgraph[5];
	SCSubgraphRef PSaskUp1 = sc.Subgraph[6];
	SCSubgraphRef PSaskUp2 = sc.Subgraph[7];
	SCSubgraphRef PSaskUp3 = sc.Subgraph[8];
	SCSubgraphRef PSaskDown1 = sc.Subgraph[9];
	SCSubgraphRef PSaskDown2 = sc.Subgraph[10];
	SCSubgraphRef PSaskDown3 = sc.Subgraph[11];
	
	SCInputRef level_1      = sc.Input[0];
	SCInputRef level_2      = sc.Input[1];
	SCInputRef level_3      = sc.Input[2];
	
	if (sc.SetDefaults)
	{
		// Set the configuration and defaults
		sc.GraphName = "Pulling/Stacking for bars";
		sc.StudyDescription = "";
		sc.AutoLoop = 0;
		sc.GraphRegion = 0;
		
		PSbidUp1.Name = "Bid Stacking 1";
		PSbidUp1.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
		PSbidUp1.PrimaryColor = RGB(0, 255, 255);
		PSbidUp1.DrawZeros = 0; 
		PSbidUp1.LineWidth = 2;
		
		PSbidUp2.Name = "Bid Stacking 2";
		PSbidUp2.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
		PSbidUp2.PrimaryColor = RGB(0, 255, 255);
		PSbidUp2.DrawZeros = 0;
		PSbidUp2.LineWidth = 2;
		
		PSbidUp3.Name = "Bid Stacking 3";
		PSbidUp3.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
		PSbidUp3.PrimaryColor = RGB(0, 255, 255);
		PSbidUp3.DrawZeros = 0;
		PSbidUp3.LineWidth = 2;
		
		PSbidDown1.Name = "Bid Pulling 1";
		PSbidDown1.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
		PSbidDown1.PrimaryColor = RGB(0, 255, 255);
		PSbidDown1.DrawZeros = 0;
		PSbidDown1.LineWidth = 2;
		
		PSbidDown2.Name = "Bid Pulling 2";
		PSbidDown2.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
		PSbidDown2.PrimaryColor = RGB(0, 255, 255);
		PSbidDown2.DrawZeros = 0;
		PSbidDown2.LineWidth = 2;
		
		PSbidDown3.Name = "Bid Pulling 3";
		PSbidDown3.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
		PSbidDown3.PrimaryColor = RGB(0, 255, 255);
		PSbidDown3.DrawZeros = 0;
		PSbidDown3.LineWidth = 2;
		
		PSaskUp1.Name = "Ask Stacking 1";
		PSaskUp1.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
		PSaskUp1.PrimaryColor = RGB(0, 255, 255);
		PSaskUp1.DrawZeros = 0;
		PSaskUp1.LineWidth = 2;
		
		PSaskUp2.Name = "Ask Stacking 3";
		PSaskUp2.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
		PSaskUp2.PrimaryColor = RGB(0, 255, 255);
		PSaskUp2.DrawZeros = 0;
		PSaskUp2.LineWidth = 2;
		
		PSaskUp3.Name = "Ask Stacking 3";
		PSaskUp3.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
		PSaskUp3.PrimaryColor = RGB(0, 255, 255);
		PSaskUp3.DrawZeros = 0;
		PSaskUp3.LineWidth = 2;
		
		PSaskDown1.Name = "Ask Pulling 1";
		PSaskDown1.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
		PSaskDown1.PrimaryColor = RGB(0, 255, 255);
		PSaskDown1.DrawZeros = 0;
		PSaskDown1.LineWidth = 2;
		
		PSaskDown2.Name = "Ask Pulling 3";
		PSaskDown2.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
		PSaskDown2.PrimaryColor = RGB(0, 255, 255);
		PSaskDown2.DrawZeros = 0;
		PSaskDown2.LineWidth = 2;
		
		PSaskDown3.Name = "Ask Pulling 3";
		PSaskDown3.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
		PSaskDown3.PrimaryColor = RGB(0, 255, 255);	
		PSaskDown3.DrawZeros = 0;
		PSaskDown3.LineWidth = 2;		
		
		level_1.Name = " % Threshold 1";
		level_1.SetInt(30);
		
		level_2.Name = " % Threshold 2";
		level_2.SetInt(60);
		
		level_3.Name = " % Threshold 3";
		level_3.SetInt(90);
		
		return;
	}
	
	int64_t& LastProcessedSequence = sc.GetPersistentInt64(1);
	int& bidPS = sc.GetPersistentInt(1);
	int& askPS = sc.GetPersistentInt(2);
	float& bidV = sc.GetPersistentFloat(1);
	float& askV = sc.GetPersistentFloat(2);
	float& firstBids = sc.GetPersistentFloat(3);
	float& firstAsks = sc.GetPersistentFloat(4);

	//reset the sequence number on a full recalculation so we start fresh for each full recalculation.
	if (sc.IsFullRecalculation && sc.UpdateStartIndex == 0)
		LastProcessedSequence = 0;

	
	// Get the Time and Sales
	c_SCTimeAndSalesArray TimeSales;
	sc.GetTimeAndSales(TimeSales);
	if (TimeSales.Size() == 0)
		return;  // No Time and Sales data available for the symbol

	if(LastProcessedSequence != 0)
	{		
		// Loop through the Time and Sales.
		for (int TSIndex = TimeSales.Size() - 1; TSIndex > 0; --TSIndex)
		{
			//do not reprocess previously processed sequence numbers.
			if (TimeSales[TSIndex].Sequence <= LastProcessedSequence)
				break;
				
			if (TimeSales[TSIndex].Type != SC_TS_MARKER)
			{
				if(TimeSales[TSIndex].Type == SC_TS_BID) bidV += TimeSales[TSIndex].Volume;
				
				if(TimeSales[TSIndex].Bid == TimeSales[TSIndex - 1].Bid)	//si este bid price y el anterior son iguales...
				{
					bidPS += TimeSales[TSIndex].BidSize - TimeSales[TSIndex - 1].BidSize;	//sumo la diferencia entre BidSizes al P/S
					if(TimeSales[TSIndex - 1].Type == SC_TS_BID) bidPS += TimeSales[TSIndex - 1].Volume;	//si ha habido una bid trade sumo su cantidad
																											//al P/S para que no se incluya en la diferencia anterior
				}
				else	//si el bid price ha cambiado...
				{
					if(TimeSales[TSIndex].Bid > TimeSales[TSIndex - 1].Bid) bidPS += TimeSales[TSIndex].BidSize;	//si el bid price ha subido, la nueva bid size
																													//la sumo al P/S: alguien la ha tenido que poner ahí
						else if(TimeSales[TSIndex - 1].Type == SC_TS_BID) bidPS -= TimeSales[TSIndex - 1].BidSize - TimeSales[TSIndex - 1].Volume;
																													//si el bid price ha bajado y ha habido trade pero de menor
																													//volumen que la última bid resto la diferencia de P/S
						else if(TimeSales[TSIndex - 1].Type != SC_TS_BID) bidPS -= TimeSales[TSIndex - 1].BidSize;	//si el bid price ha bajado pero no ha habido trade,
																													//es que la última bid se quitó y por tanto la resto de P/S
					firstBids += TimeSales[TSIndex].BidSize;	//al haber cambiado el bid price guardo la primera bid size para calcular después
				}
				
				if(TimeSales[TSIndex].Type == SC_TS_ASK) askV += TimeSales[TSIndex].Volume;
					
				if(TimeSales[TSIndex].Ask == TimeSales[TSIndex - 1].Ask)	//si este ask price y el anterior son iguales...
				{
					askPS += TimeSales[TSIndex].AskSize - TimeSales[TSIndex - 1].AskSize;	//sumo la diferencia entre AskSizes al P/S
					if(TimeSales[TSIndex - 1].Type == SC_TS_ASK) askPS += TimeSales[TSIndex - 1].Volume;	//si ha habido una ask trade sumo su cantidad
																											//al P/S para que no se incluya en la diferencia anterior
				}
				else	//si el ask price ha cambiado...
				{
					if(TimeSales[TSIndex].Ask < TimeSales[TSIndex - 1].Ask) askPS += TimeSales[TSIndex].AskSize;	//si el ask price ha bajado, la nueva ask size
																													//la sumo al P/S: alguien la ha tenido que poner ahí
						else if(TimeSales[TSIndex - 1].Type == SC_TS_ASK) askPS -= TimeSales[TSIndex - 1].AskSize - TimeSales[TSIndex - 1].Volume;
																													//si el ask price ha subido y ha habido trade pero de menor
																													//volumen que la última ask resto la diferencia de P/S
						else if(TimeSales[TSIndex - 1].Type != SC_TS_ASK) askPS -= TimeSales[TSIndex - 1].AskSize;	//si el ask price ha subido pero no ha habido trade,
																													//es que la última ask se quitó y por tanto la resto de P/S
					firstAsks += TimeSales[TSIndex].AskSize;	//al haber cambiado el ask price guardo la primera ask size para calcular después
				}

/* 				SCDateTime RecordAdjustedDateTime = TimeSales[TSIndex].DateTime;
				// Apply the time zone offset for the chart. This will result in the actual date-time of the record in the charts time zone.
				RecordAdjustedDateTime += sc.TimeScaleAdjustment;  */ 
			}
		}
	}
	
	LastProcessedSequence = TimeSales[TimeSales.Size() - 1].Sequence;
	
	float bidRatio = bidPS / bidV * 100, askRatio = askPS / askV * 100;
	int lastBar = sc.UpdateStartIndex;
	if(bidRatio > level_1.GetInt()) PSbidUp1[sc.UpdateStartIndex] = sc.Low[sc.UpdateStartIndex] - 2 * sc.TickSize;
		else PSbidUp1[sc.UpdateStartIndex] = 0;
	if(bidRatio > level_2.GetInt()) PSbidUp2[sc.UpdateStartIndex] = sc.Low[sc.UpdateStartIndex] - 3 * sc.TickSize;
		else PSbidUp2[sc.UpdateStartIndex] = 0;
	if(bidRatio > level_3.GetInt()) PSbidUp3[sc.UpdateStartIndex] = sc.Low[sc.UpdateStartIndex] - 4 * sc.TickSize;
		else PSbidUp3[sc.UpdateStartIndex] = 0;
	if(bidRatio < -level_1.GetInt()) PSbidDown1[sc.UpdateStartIndex] = sc.Low[sc.UpdateStartIndex] - 2 * sc.TickSize;
		else PSbidDown1[sc.UpdateStartIndex] = 0;
	if(bidRatio < -level_2.GetInt()) PSbidDown2[sc.UpdateStartIndex] = sc.Low[sc.UpdateStartIndex] - 3 * sc.TickSize;
		else PSbidDown2[sc.UpdateStartIndex] = 0;
	if(bidRatio < -level_3.GetInt()) PSbidDown3[sc.UpdateStartIndex] = sc.Low[sc.UpdateStartIndex] - 4 * sc.TickSize;
		else PSbidDown3[sc.UpdateStartIndex] = 0;

	if(askRatio > level_1.GetInt()) PSaskUp1[sc.UpdateStartIndex] = sc.High[sc.UpdateStartIndex] + 2 * sc.TickSize;
		else PSaskUp1[sc.UpdateStartIndex] = 0;
	if(askRatio > level_2.GetInt()) PSaskUp2[sc.UpdateStartIndex] = sc.High[sc.UpdateStartIndex] + 3 * sc.TickSize;
		else PSaskUp2[sc.UpdateStartIndex] = 0;
	if(askRatio > level_3.GetInt()) PSaskUp3[sc.UpdateStartIndex] = sc.High[sc.UpdateStartIndex] + 4 * sc.TickSize;
		else PSaskUp3[sc.UpdateStartIndex] = 0;
	if(askRatio < -level_1.GetInt()) PSaskDown1[sc.UpdateStartIndex] = sc.High[sc.UpdateStartIndex] + 2 * sc.TickSize;
		else PSaskDown1[sc.UpdateStartIndex] = 0;
	if(askRatio < -level_2.GetInt()) PSaskDown2[sc.UpdateStartIndex] = sc.High[sc.UpdateStartIndex] + 3 * sc.TickSize;
		else PSaskDown2[sc.UpdateStartIndex] = 0;
	if(askRatio < -level_3.GetInt()) PSaskDown3[sc.UpdateStartIndex] = sc.High[sc.UpdateStartIndex] + 4 * sc.TickSize;
		else PSaskDown3[sc.UpdateStartIndex] = 0;
	
	if(sc.GetBarHasClosedStatus(sc.Index) == BHCS_BAR_HAS_CLOSED)
	{
		askPS = 0; bidPS = 0;
		bidV = 0; askV = 0;
		firstBids = 0; firstAsks = 0;
	}
}


SCSFExport scsf_StopsOnChart(SCStudyInterfaceRef sc)
{
	SCSubgraphRef StopBuys = sc.Subgraph[0];
	SCSubgraphRef StopSells = sc.Subgraph[1];
	
	SCInputRef nTicks	= sc.Input[0];
	
	if (sc.SetDefaults)
	{
		// Set the configuration and defaults
		sc.GraphName = "Stops drawing";
		sc.StudyDescription = "";
		sc.AutoLoop = 0;
		sc.GraphRegion = 0;
		
		StopBuys.Name = "Stopfor Buys";
		StopBuys.DrawStyle = DRAWSTYLE_RIGHT_SIDE_TICK_SIZE_RECTANGLE;
		StopBuys.PrimaryColor = RGB(0, 255, 255);
		StopBuys.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN | LL_NAME_REVERSE_COLORS;
		StopBuys.ShortName = "STOP";
		StopBuys.DrawZeros = 0; 
		StopBuys.LineWidth = 2;
		
		StopSells.Name = "Stopfor Buys";
		StopSells.DrawStyle = DRAWSTYLE_RIGHT_SIDE_TICK_SIZE_RECTANGLE;
		StopSells.PrimaryColor = RGB(0, 255, 255);
		StopSells.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN | LL_NAME_REVERSE_COLORS;
		StopSells.ShortName = "STOP";
		StopSells.DrawZeros = 0; 
		StopSells.LineWidth = 2;
		
		nTicks.Name = "Stop in ticks";
		nTicks.SetInt(12);
	}
	
	int& lastIndex = sc.GetPersistentInt(0);
	int i = sc.UpdateStartIndex;
	
	//on start, set subgraphs names
	if (sc.IsFullRecalculation && i == 0)
	{
		StopBuys.ShortName.Format("-%i T", nTicks.GetInt() );
		StopSells.ShortName.Format("%i T", nTicks.GetInt() );
	}
	
	//if new candle, copy last values
	if(sc.UpdateStartIndex != lastIndex)
	{
		StopSells[i] = StopSells[i-1];
		StopBuys[i] = StopBuys[i-1];
		StopSells[i-1] = 0; 
		StopBuys[i-1] = 0;
	}
	
	
	StopSells[i] = sc.Bid + nTicks.GetInt() * sc.TickSize;
	StopBuys[i] = sc.Ask - nTicks.GetInt() * sc.TickSize;
	
	lastIndex = i;
}

		
SCSFExport scsf_BetterDOM(SCStudyInterfaceRef sc)
{
	//old hi&los will be automatically created and numbered from subgraph[40] backwards, copying the properties
	//specified for the prevHis and prevLos subgraphs ([42] and [43])
	SCSubgraphRef hiGraph = sc.Subgraph[40];
	SCSubgraphRef loGraph = sc.Subgraph[41];
	SCSubgraphRef prevHisGraphs = sc.Subgraph[42];
	SCSubgraphRef prevLosGraphs = sc.Subgraph[43];

	SCSubgraphRef swingDataGraph = sc.Subgraph[55];		
	SCSubgraphRef askSizeGraph = sc.Subgraph[56];
	SCSubgraphRef bidSizeGraph = sc.Subgraph[57];
	SCSubgraphRef cumDeltaGraph = sc.Subgraph[58];
	SCSubgraphRef cumSizeGraph = sc.Subgraph[59];	
	
	SCInputRef nTicks	= sc.Input[0];
	SCInputRef clear	= sc.Input[1];
	SCInputRef nHiLos = sc.Input[2];		
	SCInputRef showSizes = sc.Input[3];
	SCInputRef cumSizeColoring = sc.Input[4];		
	SCInputRef whereDisplayData = sc.Input[5];
	SCInputRef dataToDisplay = sc.Input[6];
	SCInputRef deltaType = sc.Input[7];
	SCInputRef showMaxMin = sc.Input[8];	
	SCInputRef noDataDisplay = sc.Input[9];
	SCInputRef interpColorsOldHiLos = sc.Input[10];
	SCInputRef opaqueBackground = sc.Input[11];
	SCInputRef colorSwingData = sc.Input[12];
	SCInputRef fontSizeSwingData = sc.Input[13];
	SCInputRef fontSizeLastHiLo = sc.Input[14];
	SCInputRef fontSizeOldHiLos = sc.Input[15];
	SCInputRef spacing	= sc.Input[16];
	
	//0 decimal places for volume data display
	sc.ValueFormat = 0;
		
	if (sc.SetDefaults)
	{
		//SUBGRAPHS
		sc.GraphName = "Better DOM";
		sc.StudyDescription = "Indicator shows new swing Highs and Lows on the DOM label column along with volume, delta and cumulative pulling&stacking values for each swing";
		sc.AutoLoop = 0; 
		sc.GraphRegion = 0;	
		
		hiGraph.Name = "Last High";
		hiGraph.DrawStyle = DRAWSTYLE_TRANSPARENT_TEXT;
		hiGraph.LineWidth = 10;
		hiGraph.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN | LL_NAME_REVERSE_COLORS;
		hiGraph.PrimaryColor = RGB(255, 255, 0);
		hiGraph.DrawZeros = 0;
		hiGraph.ShortName = "HI";
		 
		loGraph.Name = "Last Low";
		loGraph.DrawStyle = DRAWSTYLE_TRANSPARENT_TEXT;
		loGraph.LineWidth = 10;
		loGraph.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN | LL_NAME_REVERSE_COLORS;
		loGraph.PrimaryColor = RGB(255, 255, 0);
		loGraph.DrawZeros = 0;
		loGraph.ShortName = "LO";
		
		cumSizeGraph.Name = "Cumulative Size";
		cumSizeGraph.DrawStyle	= DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
		cumSizeGraph.LineLabel = LL_DISPLAY_VALUE | LL_VALUE_ALIGN_DOM_LABELS_COLUMN | LL_DISPLAY_CUSTOM_VALUE_AT_Y | LL_VALUE_REVERSE_COLORS_INV;
		cumSizeGraph.PrimaryColor = RGB(255,255,255); 
		
		cumDeltaGraph.Name = "% Delta per Price";
		cumDeltaGraph.DrawStyle	= DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
		cumDeltaGraph.LineLabel = LL_DISPLAY_VALUE | LL_VALUE_ALIGN_DOM_LABELS_COLUMN | LL_DISPLAY_CUSTOM_VALUE_AT_Y | LL_VALUE_REVERSE_COLORS_INV;
		cumDeltaGraph.PrimaryColor = RGB(100, 255, 100);
		cumDeltaGraph.SecondaryColor = RGB(255, 100, 100);
		cumDeltaGraph.SecondaryColorUsed = 1;
		cumDeltaGraph.AutoColoring = AUTOCOLOR_POSNEG;
		
		askSizeGraph.Name = "Recent Ask Size/Up color";
		askSizeGraph.DrawStyle	= DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
		askSizeGraph.LineLabel = LL_DISPLAY_VALUE | LL_VALUE_ALIGN_DOM_LABELS_COLUMN | LL_DISPLAY_CUSTOM_VALUE_AT_Y | LL_VALUE_REVERSE_COLORS_INV;
		askSizeGraph.PrimaryColor = RGB(100,255,100);
		
		bidSizeGraph.Name = "Recent Bid Size/Down color";
		bidSizeGraph.DrawStyle	= DRAWSTYLE_SUBGRAPH_NAME_AND_VALUE_LABELS_ONLY;
		bidSizeGraph.LineLabel = LL_DISPLAY_VALUE | LL_VALUE_ALIGN_DOM_LABELS_COLUMN | LL_DISPLAY_CUSTOM_VALUE_AT_Y | LL_VALUE_REVERSE_COLORS_INV;
		bidSizeGraph.PrimaryColor = RGB(255,100,100);
		
		swingDataGraph.Name = "Current Swing's Data";
		swingDataGraph.DrawStyle = DRAWSTYLE_TEXT;
		swingDataGraph.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN | LL_NAME_REVERSE_COLORS;
		swingDataGraph.LineWidth = 10;
		swingDataGraph.PrimaryColor = RGB(255, 255, 255);
			
		prevHisGraphs.Name = "Previous Highs";
		prevHisGraphs.DrawStyle	= DRAWSTYLE_TRANSPARENT_TEXT;
		prevHisGraphs.LineWidth = 10;
		prevHisGraphs.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN;
		prevHisGraphs.PrimaryColor = RGB(200, 0, 0);
		prevHisGraphs.SecondaryColor = RGB(100, 0, 0);
		prevHisGraphs.SecondaryColorUsed = 1;
		prevHisGraphs.DrawZeros = 0;
		prevHisGraphs.ShortName = "hi";
		
		prevLosGraphs.Name = "Previous Lows";
		prevLosGraphs.DrawStyle	= DRAWSTYLE_TRANSPARENT_TEXT;
		prevLosGraphs.LineWidth = 10;
		prevLosGraphs.LineLabel = LL_DISPLAY_NAME | LL_NAME_ALIGN_DOM_LABELS_COLUMN;
		prevLosGraphs.PrimaryColor = RGB(0, 200, 0);
		prevLosGraphs.SecondaryColor = RGB(0, 100, 0);
		prevLosGraphs.SecondaryColorUsed = 1;
		prevLosGraphs.DrawZeros = 0;
		prevLosGraphs.ShortName = "lo";

		
		// INPUTS
		nTicks.Name = "Number of ticks to confirm new swing Hi&Lo";
		nTicks.SetInt(12);
		
		clear.Name = "Clear Current Trades on new confirmed Hi/Lo";
		clear.SetYesNo(1);
		
		showSizes.Name = "Show Last Sizes";
		showSizes.SetCustomInputStrings("No;Cumulative Last Size;Cum.Size + DeltaPerPrice;Cum.Size + DeltaPerPrice as %;Bid/Ask sizes");
		showSizes.SetCustomInputIndex(4);
		
		cumSizeColoring.Name = "Cumulative Size Coloring";
		cumSizeColoring.SetCustomInputStrings("Subgraph color;Uptick/Downtick;Delta per Price");
		cumSizeColoring.SetCustomInputIndex(2);
		
		whereDisplayData.Name = "Where to display swing data";
		whereDisplayData.SetCustomInputStrings("None;Only Current Swing;Curr.Swing and Only Last High & Low;Curr.Swing and All Highs and Lows");
		whereDisplayData.SetCustomInputIndex(2);
		
		opaqueBackground.Name = "Opaque text background for data display in";
		opaqueBackground.SetCustomInputStrings("None;Only Current Swing;Curr.Swing and Only Last High & Low;Curr.Swing and All Highs and Lows");
		opaqueBackground.SetCustomInputIndex(0);
		
		noDataDisplay.Name = "Display Highs & Lows with no data as";
		noDataDisplay.SetCustomInputStrings("Set in Subgraphs tab;Up/Down Triangles of fontSize");
		noDataDisplay.SetCustomInputIndex(1);
		
		dataToDisplay.Name = "Swing data to display";
		dataToDisplay.SetCustomInputStrings("Volume;V + Delta;V + D + Pulling&Stacking");
		dataToDisplay.SetCustomInputIndex(2);
		
		deltaType.Name = "Delta Format";
		deltaType.SetCustomInputStrings("Absolute Value;Volume Percentage");
		deltaType.SetCustomInputIndex(0);
		
		spacing.Name = "Vertical spacing between data in ticks";
		spacing.SetInt(1);
		
		nHiLos.Name = "Number of previous Hi&Los to display";
		nHiLos.SetInt(5);
		
		interpColorsOldHiLos.Name = "Interpolate Primary/Secondary colors for Old Hi&Los";
		interpColorsOldHiLos.SetYesNo(1);
		
		showMaxMin.Name = "Show current swing's max/min Delta and P&S values";
		showMaxMin.SetYesNo(1);
		
		colorSwingData.Name = "Color Swing Data acc. to swing delta (w Bid/Ask colors)";
		colorSwingData.SetYesNo(1);
		
		fontSizeSwingData.Name = "Font size for swing data";
		fontSizeSwingData.SetInt(12);
		
		fontSizeLastHiLo.Name = "Font size for Last High and Low";
		fontSizeLastHiLo.SetInt(10);
		
		fontSizeOldHiLos.Name = "Font size for Old Highs and Lows";
		fontSizeOldHiLos.SetInt(8);
	}
	
	//SET REFERENCES TO PERSISTENT VARIABLES
	float& newLo = sc.GetPersistentFloat(0);
	float& newHi = sc.GetPersistentFloat(1);
	float& lastTradedPrice = sc.GetPersistentFloat(2);
	float& lastTradedBid = sc.GetPersistentFloat(3);
	float& lastTradedAsk = sc.GetPersistentFloat(4);
	float& lastAsk = sc.GetPersistentFloat(5);
	float& lastBid = sc.GetPersistentFloat(6);

	int& lastIndex = sc.GetPersistentInt(0);
	int& swingPS = sc.GetPersistentInt(1);
	int& newPS = sc.GetPersistentInt(2);
	int& swingMaxPS = sc.GetPersistentInt(3);
	int& swingMinPS = sc.GetPersistentInt(4);
	int& newMaxPS = sc.GetPersistentInt(5);
	int& newMinPS = sc.GetPersistentInt(6);
	int& origColorCumSize = sc.GetPersistentInt(7);
	int& cmB = sc.GetPersistentInt(8);
	int& cmA = sc.GetPersistentInt(9);
	int& swingVol = sc.GetPersistentInt(10);
	int& swingDelta = sc.GetPersistentInt(11);
	int& newVol = sc.GetPersistentInt(12);
	int& newDelta = sc.GetPersistentInt(13);
	int& swingMaxDelta = sc.GetPersistentInt(14);
	int& swingMinDelta = sc.GetPersistentInt(15);
	int& newMaxDelta = sc.GetPersistentInt(16);
	int& newMinDelta = sc.GetPersistentInt(17);
	
	int64_t& LastProcessedSequence = sc.GetPersistentInt64(0);
	
	//set some variables
	int i = sc.UpdateStartIndex;
	float lineSpacing = spacing.GetInt() * sc.TickSize;

	// Get the Time and Sales
	c_SCTimeAndSalesArray TimeSales;
	sc.GetTimeAndSales(TimeSales);
	if (TimeSales.Size() == 0)
		return;  // No Time and Sales data available for the symbol
		
	//on start, configure all subgraphs acc. to inputs and create enough subgraphs for nHiLos to display 
	if (sc.IsFullRecalculation && i == 0)
	{
		//save original color for the cumulative size subgraph if it is set with tick up/down coloring
		if(cumSizeColoring.GetIndex() == 1) origColorCumSize = cumSizeGraph.PrimaryColor;
		
		//set highs and lows subgraphs
		if(noDataDisplay.GetIndex() > 0)
		{
			prevHisGraphs.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
			prevHisGraphs.LineWidth = fontSizeOldHiLos.GetInt();
			prevLosGraphs.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
			prevLosGraphs.LineWidth = fontSizeOldHiLos.GetInt();
			
			hiGraph.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
			hiGraph.LineWidth = fontSizeLastHiLo.GetInt();
			loGraph.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
			loGraph.LineWidth = fontSizeLastHiLo.GetInt();
		}
		
		if(whereDisplayData.GetIndex() >= 2)
		{
			hiGraph.DrawStyle = opaqueBackground.GetIndex() >= 2 ? DRAWSTYLE_TEXT : DRAWSTYLE_TRANSPARENT_TEXT;
			hiGraph.LineWidth = fontSizeLastHiLo.GetInt();
			loGraph.DrawStyle = opaqueBackground.GetIndex() >= 2 ? DRAWSTYLE_TEXT : DRAWSTYLE_TRANSPARENT_TEXT;
			loGraph.LineWidth = fontSizeLastHiLo.GetInt();
			if(whereDisplayData.GetIndex() == 3)
				{
					prevHisGraphs.DrawStyle = opaqueBackground.GetIndex() == 3 ? DRAWSTYLE_TEXT : DRAWSTYLE_TRANSPARENT_TEXT;
					prevHisGraphs.LineWidth = fontSizeOldHiLos.GetInt();
					prevLosGraphs.DrawStyle = opaqueBackground.GetIndex() == 3 ? DRAWSTYLE_TEXT : DRAWSTYLE_TRANSPARENT_TEXT;
					prevLosGraphs.LineWidth = fontSizeOldHiLos.GetInt();
				}
		}
		
		//set current swing data subgraph
		if(whereDisplayData.GetIndex() > 0)
		{
			swingDataGraph.DrawStyle = opaqueBackground.GetIndex() >= 1 ? DRAWSTYLE_TEXT : DRAWSTYLE_TRANSPARENT_TEXT;
			swingDataGraph.LineWidth = fontSizeSwingData.GetInt();
		}
		
		//create enough subgraphs for old Highs and lows on the basis of the "previous" subgraphs
		for(int x = 1; x < 21; x++)
		{
			if(x < nHiLos.GetInt() )
			{
				//new subgraphs for previous highs
				sc.Subgraph[40 - 2*x].Name.Format("High %i", x);
				sc.Subgraph[40 - 2*x].DrawStyle = prevHisGraphs.DrawStyle;
				sc.Subgraph[40 - 2*x].LineWidth = prevHisGraphs.LineWidth;
				sc.Subgraph[40 - 2*x].LineLabel = prevHisGraphs.LineLabel;
				sc.Subgraph[40 - 2*x].PrimaryColor =
					interpColorsOldHiLos.GetYesNo() ? sc.RGBInterpolate(prevHisGraphs.PrimaryColor, prevHisGraphs.SecondaryColor, (float)(x-1) / (nHiLos.GetInt() - 2) )
					: prevHisGraphs.PrimaryColor;
				sc.Subgraph[40 - 2*x].DrawZeros = 0;
				sc.Subgraph[40 - 2*x].ShortName.Format(prevHisGraphs.ShortName + " %i", x);
				
				//new subgraphs for previous lows
				sc.Subgraph[41 - 2*x].Name.Format("Low %i", x);
				sc.Subgraph[41 - 2*x].DrawStyle = prevLosGraphs.DrawStyle;
				sc.Subgraph[41 - 2*x].LineWidth = prevLosGraphs.LineWidth;
				sc.Subgraph[41 - 2*x].LineLabel = prevLosGraphs.LineLabel;
				sc.Subgraph[41 - 2*x].PrimaryColor =
					interpColorsOldHiLos.GetYesNo() ? sc.RGBInterpolate(prevLosGraphs.PrimaryColor, prevLosGraphs.SecondaryColor, (float)(x-1) / (nHiLos.GetInt() - 2) )
					: prevLosGraphs.PrimaryColor;
				sc.Subgraph[41 - 2*x].DrawZeros = 0;
				sc.Subgraph[41 - 2*x].ShortName.Format(prevLosGraphs.ShortName + " %i", x);
			}
			else
			{
				//do not show possible hi&los subgraphs if we are now showing a lesser number of them
				sc.Subgraph[40 - 2*x].Name = "";
				sc.Subgraph[41 - 2*x].Name = "";
			}			
		}
	
		//set on first calculation
		LastProcessedSequence = 0;
		lastIndex = sc.ArraySize-1;
		newLo = TimeSales[0].Bid;
		newHi = 0;
		//set name for swing data subgraph's label: it will be making new possible lows
		swingDataGraph.ShortName = "t.lo";
	}
	
	//calculate only in real time
	else
	{
		//if new candle, copy last subgraph values
		if(i != lastIndex)
			for(int x = 0; x < 60; x++)
			{
				sc.Subgraph[x].Data[i] = sc.Subgraph[x].Data[i-1];
				sc.Subgraph[x].Arrays[0][i] = sc.Subgraph[x].Arrays[0][i-1];
				sc.Subgraph[x].Data[i-1] = 0;
				sc.Subgraph[x].Arrays[0][i-1] = 0;
			}

		
		//READ T&S FOR VOLUME, DELTA AND PULLING & STACKING
		//first find first to calculate in this update
		int TSIndex = TimeSales.Size() - 1;
		for(TSIndex; TSIndex > 0; TSIndex--)
		{
			if (TimeSales[TSIndex - 1].Sequence <= LastProcessedSequence)
				break;
		}
		
		//now, iterate over the Time&Sales array
		int bidV = 0, askV = 0;
		float updateTradedPrice = 0, updateTradedBid = 0, updateTradedAsk = 0;

		for (TSIndex; TSIndex < TimeSales.Size(); TSIndex++)
		{		
			//flag to check if price has changed in this iteration
			float prevPrice = updateTradedPrice;
			
			//ADD UP ALL BID VOLUME DURING THIS UPDATE AND SAVE LAST PRICES
			if(TimeSales[TSIndex].Type == SC_TS_BID)
			{
				bidV += TimeSales[TSIndex].Volume;
				updateTradedBid = TimeSales[TSIndex].Bid;
				updateTradedPrice = updateTradedBid;
				
				swingVol += TimeSales[TSIndex].Volume;
				newVol += TimeSales[TSIndex].Volume;
				swingDelta -= TimeSales[TSIndex].Volume;
				newDelta -= TimeSales[TSIndex].Volume;
			}
			
			//ADD UP ALL BID VOLUME DURING THIS UPDATE AND SAVE LAST PRICES
			if(TimeSales[TSIndex].Type == SC_TS_ASK)
			{
				askV += TimeSales[TSIndex].Volume;
				updateTradedAsk = TimeSales[TSIndex].Ask;
				updateTradedPrice = updateTradedAsk;
				
				swingVol += TimeSales[TSIndex].Volume;
				newVol += TimeSales[TSIndex].Volume;
				swingDelta += TimeSales[TSIndex].Volume;
				newDelta += TimeSales[TSIndex].Volume;
			}
			
			//calculate max&min delta for this swing and for potential new swing
			//if(maxMinDeltaGraph.DrawStyle != DRAWSTYLE_HIDDEN && maxMinDeltaGraph.DrawStyle != DRAWSTYLE_IGNORE)
			if(showMaxMin.GetYesNo() )
				if(TimeSales[TSIndex].Type == SC_TS_ASK || TimeSales[TSIndex].Type == SC_TS_BID)
				{
					if(swingDelta > swingMaxDelta || swingMaxDelta == 0) swingMaxDelta = swingDelta;
					if(swingDelta < swingMinDelta || swingMinDelta == 0) swingMinDelta = swingDelta;
					if(newDelta > newMaxDelta || newMaxDelta == 0) newMaxDelta = newDelta;
					if(newDelta < newMinDelta || newMinDelta == 0) newMinDelta = newDelta;
				}
			
			//PULLING & STACKING
			//if(swingPSGraph.DrawStyle != DRAWSTYLE_HIDDEN && swingPSGraph.DrawStyle != DRAWSTYLE_IGNORE)
			if(dataToDisplay.GetIndex() == 2)
				if(TimeSales[TSIndex].Type == SC_TS_BIDASKVALUES && TimeSales[TSIndex - 1].Type == SC_TS_BIDASKVALUES)
				{
					int ps = 0;
					
					if(TimeSales[TSIndex].Bid == TimeSales[TSIndex - 1].Bid)
						ps += (TimeSales[TSIndex].BidSize - TimeSales[TSIndex - 1].BidSize);
					else if(TimeSales[TSIndex].Bid > TimeSales[TSIndex - 1].Bid) ps += TimeSales[TSIndex].BidSize;
					else ps -= TimeSales[TSIndex - 1].BidSize;
					
					if(TimeSales[TSIndex].Ask == TimeSales[TSIndex - 1].Ask)
						ps -= (TimeSales[TSIndex].AskSize - TimeSales[TSIndex - 1].AskSize);
					else if(TimeSales[TSIndex].Ask < TimeSales[TSIndex - 1].Ask) ps -= TimeSales[TSIndex].AskSize;
					else ps += TimeSales[TSIndex - 1].AskSize;
					
					swingPS += ps;
					newPS += ps;
					
					//calculate max&min P&S values for this swing and for potential new swing
					//if(maxMinPSGraph.DrawStyle != DRAWSTYLE_HIDDEN && maxMinPSGraph.DrawStyle != DRAWSTYLE_IGNORE)
					if(showMaxMin.GetYesNo() )
					{
						if(swingPS > swingMaxPS || swingMaxPS == 0) swingMaxPS = swingPS;
						if(swingPS < swingMinPS || swingMinPS == 0) swingMinPS = swingPS; 
						if(newPS > newMaxPS || newMaxPS == 0) newMaxPS = newPS;
						if(newPS < newMinPS || newMinPS == 0) newMinPS = newPS;
					}
				}


			//HAS A NEW HIGH OR LOW BEEN CONFIRMED?
			if(updateTradedPrice != prevPrice && updateTradedPrice != 0)
			{
				//if there is a new confirmed high
				if(updateTradedPrice <= (newHi - nTicks.GetInt() * sc.TickSize) && newLo == 0 && newHi != 0)
				{
					//new confirmed Hi
					//move highs to previous highs subgraphs and change their number display
					for(int x = nHiLos.GetInt() - 1; x > 0 ; x--)
					{
						sc.Subgraph[40 - 2 * x][i] = sc.Subgraph[40 - 2*(x - 1)][i];
						SCString prevHiLoData = sc.Subgraph[40 - 2*(x - 1)].TextDrawStyleText;					
						sc.Subgraph[40 - 2 * x].TextDrawStyleText.Format("high %i%s", x, prevHiLoData.GetSubString(100, prevHiLoData.IndexOf('\n', 0) ).GetChars() );
					}

					//set subgraph
					hiGraph[i] = newHi;
					SCString swingData = "HIGH";

					swingData.AppendFormat("\nV: %i", swingVol - newVol);
					if(dataToDisplay.GetIndex() > 0)
					{
						if(deltaType.GetIndex() == 0)
							swingData.AppendFormat("\nD: %i", swingDelta - newDelta);
						else 
							swingData.AppendFormat("\nD: %i%%", (int) ( 100 * (swingDelta - newDelta) / ((float)(swingVol - newVol) ) ) );
						if(dataToDisplay.GetIndex() == 2)
							swingData.AppendFormat("\nPS: %i", swingPS - newPS);
					}

					hiGraph.TextDrawStyleText = swingData;
					
					//new swing starts: new swing's values count from the now confirmed high
					swingVol = newVol;
					swingDelta = newDelta;
					swingMaxDelta = newMaxDelta;
					swingMinDelta = newMinDelta;
					swingPS = newPS;
					swingMaxPS = newMaxPS;
					swingMinPS = newMinPS;
					
					//reset "new" variables to look for next lo
					newLo = updateTradedPrice;
					newHi = 0;
					newVol = 0;
					newDelta = 0;
					newMaxDelta = 0;
					newMinDelta = 0;
					newPS = 0;
					newMaxPS = 0;
					newMinPS = 0;
					
					//set name for swing data subgraph's label: it will be making new possible lows
					swingDataGraph.ShortName = "t.lo";
					
					//clear current trades
					if(clear.GetYesNo() ) sc.ClearCurrentTradedBidAskVolume();
				}
				else
				//if there is a new confirmed low
				if(updateTradedPrice >= (newLo + nTicks.GetInt() * sc.TickSize) && newHi == 0 && newLo != 0)
				{
					//new confirmed Lo
					for(int x = nHiLos.GetInt() - 1; x > 0 ; x--)
					{
						sc.Subgraph[41 - 2 * x][i] = sc.Subgraph[41 - 2*(x - 1)][i];
 						SCString prevHiLoData = sc.Subgraph[41 - 2*(x - 1)].TextDrawStyleText;					
						sc.Subgraph[41 - 2 * x].TextDrawStyleText.Format("low %i%s", x, prevHiLoData.GetSubString(100, prevHiLoData.IndexOf('\n', 0) ).GetChars() );
					}
					//set subgraph
					loGraph[i] = newLo;
					SCString swingData = "LOW";

					swingData.AppendFormat("\nV: %i", swingVol - newVol);
					if(dataToDisplay.GetIndex() > 0)
					{
						if(deltaType.GetIndex() == 0)
							swingData.AppendFormat("\nD: %i", swingDelta - newDelta);
						else 
							swingData.AppendFormat("\nD: %i%%", (int) ( 100 * (swingDelta - newDelta) / ((float)(swingVol - newVol) ) ) );
						if(dataToDisplay.GetIndex() == 2)
							swingData.AppendFormat("\nPS: %i", swingPS - newPS);
					}

					loGraph.TextDrawStyleText = swingData;
					
					//new swing: new values count from the now confirmed low
					swingVol = newVol;
					swingDelta = newDelta;
					swingMaxDelta = newMaxDelta;
					swingMinDelta = newMinDelta;
					swingPS = newPS;
					swingMaxPS = newMaxPS;
					swingMinPS = newMinPS;

					//reset "new" variables to look for next high
					newHi = updateTradedPrice;
					newLo = 0;
					newVol = 0;
					newDelta = 0;
					newMaxDelta = 0;
					newMinDelta = 0;
					newPS = 0;
					newMaxPS = 0;
					newMinPS = 0;
					
					//set name for swing data subgraph's label: it will be making new possible highs
					swingDataGraph.ShortName = "t.hi";
					
					//clear current trades
					if(clear.GetYesNo() ) sc.ClearCurrentTradedBidAskVolume();
				}
				else
				//now searching for next high or low and resetting new volume, delta and PS values each time
				//a potential new high or low is not confirmed
				if(newLo != 0 && updateTradedPrice < newLo)
				{
					newLo = updateTradedPrice;
					newVol = 0;
					newDelta = 0;
					newMaxDelta = 0;
					newMinDelta = 0;
					newPS = 0;
					newMaxPS = 0;
					newMinPS = 0;
				}
				else
				if(newHi != 0 && updateTradedPrice > newHi)
				{
					newHi = updateTradedPrice;
					newVol = 0;
					newDelta = 0;
					newMaxDelta = 0;
					newMinDelta = 0;
					newPS = 0;
					newMaxPS = 0;
					newMinPS = 0;
				}
			}
		}	
		//---END OF TIME & SALES LOOP
		
		
		//after looping over Time&Sales, if there have been any trades, updateTradedPrice will be != 0, then get the trades sizes and display them
		if(updateTradedPrice != 0)
		{
			//CUMULATIVE SIZE AT PRICE
			if(showSizes.GetIndex() >= 1 && showSizes.GetIndex() <= 3)
			{
				if(updateTradedPrice != lastTradedPrice)
				{
					cmB = 0;
					cmA = 0;	
				}
			
				cmB += bidV;
				cmA += askV;
			
				cumSizeGraph.Data[i] = cmB + cmA;
				cumSizeGraph.Arrays[0][i] = updateTradedPrice;
				
				//coloring mode for last cumulative size
				if(cumSizeColoring.GetIndex() == 1)
				{
					//cumSizeGraph.PrimaryColor = updateTradedPrice == lastTradedPrice ? origColorCumSize : updateTradedPrice > lastTradedPrice ? askSizeGraph.PrimaryColor : bidSizeGraph.PrimaryColor;
					updateTradedBid = sc.Bid;
					updateTradedAsk = sc.Ask;
					
					cumSizeGraph.PrimaryColor = (updateTradedBid > lastTradedBid && updateTradedAsk > lastTradedAsk) ? askSizeGraph.PrimaryColor : (updateTradedBid < lastTradedBid && updateTradedAsk < lastTradedAsk) ? bidSizeGraph.PrimaryColor : origColorCumSize;
					lastTradedBid = updateTradedBid;
					lastTradedAsk = updateTradedAsk;
				}
				else if(cumSizeColoring.GetIndex() == 2)
					cumSizeGraph.PrimaryColor = cmA - cmB > 0 ? cumDeltaGraph.PrimaryColor : cumDeltaGraph.SecondaryColor;
				
				//if showing delta per price
				if(showSizes.GetIndex() >= 2)
				{
					cumDeltaGraph.Data[i] = cmA - cmB;
					//if expressing delta per price as percentage
					if(showSizes.GetIndex() == 3)
					{
						cumDeltaGraph.Data[i] /= (cmA + cmB);
						cumDeltaGraph.Data[i] *= 100;
					}
					cumDeltaGraph.Arrays[0][i] = updateTradedPrice - lineSpacing;
				}
			lastTradedPrice = updateTradedPrice;
			}
			
			//SHOW BID & ASK SIZES
			else if(showSizes.GetIndex() == 4)
			{
				if(bidV != 0)
				{
					if(updateTradedBid != lastTradedBid) bidSizeGraph.Data[i] = 0;
					bidSizeGraph.Data[i] += bidV;
					bidSizeGraph.Arrays[0][i] = sc.Bid;
					lastTradedBid = updateTradedBid;
				}
				
				if(askV != 0)
				{
				if(updateTradedAsk != lastTradedAsk) askSizeGraph.Data[i] = 0;
				askSizeGraph.Data[i] += askV;
				askSizeGraph.Arrays[0][i] = spacing.GetInt() > 1 ? sc.Bid + lineSpacing : sc.Ask;
				lastTradedAsk = updateTradedAsk;
				}	
			}
		}
		
		//WRITE CURRENT SWING VALUES
		SCString swingData = "";
		if(whereDisplayData.GetIndex() > 0)
		{
			swingData = newLo != 0 ? "temp LO" : "temp HI";
			swingData.AppendFormat("\nV: %i", swingVol);
			if(dataToDisplay.GetIndex() > 0)
			{
				swingData.Append("\n------------");
				if(deltaType.GetIndex() == 0)
				{
					swingData.AppendFormat("\nDELTA: %i", swingDelta);
					if(showMaxMin.GetYesNo())
						swingData.AppendFormat("\ndmax: %i\ndmin: %i", swingMaxDelta, swingMinDelta);
				}
				else 
				{
					swingData.AppendFormat("\nDELTA: %i%%", (int) ( 100 * swingDelta/ ((float) swingVol ) ) );
					if(showMaxMin.GetYesNo())
						swingData.AppendFormat("\ndmax: %i%%\ndmin: %i%%", (int)(100 * swingMaxDelta /((float) swingVol) ), (int)(100 * swingMinDelta /((float) swingVol) ) );
				}
				if(dataToDisplay.GetIndex() == 2)
				{
					swingData.Append("\n------------");
					swingData.AppendFormat("\nPL&ST: %i", swingPS);
					if(showMaxMin.GetYesNo())
						swingData.AppendFormat("\npsmax: %i\npsmin: %i", swingMaxPS, swingMinPS);
				}
			}
		}
		swingDataGraph.TextDrawStyleText = swingData;
		swingDataGraph[i] = newLo == 0 && newHi == 0 ? sc.Ask : newLo != 0 ? newLo : newHi;
		if(colorSwingData.GetYesNo() )
			swingDataGraph.PrimaryColor = swingDelta > 0 ? askSizeGraph.PrimaryColor : bidSizeGraph.PrimaryColor;


		//SAVE VALUES FOR NEXT UPDATE
		LastProcessedSequence = TimeSales[TimeSales.Size() - 1].Sequence;
		//save candle index
		lastIndex = i;
	}
}

SCSFExport scsf_ColorDeltaBars(SCStudyInterfaceRef sc)
{
	SCSubgraphRef ColorDeltaBar = sc.Subgraph[0];
	SCSubgraphRef AskPOC = sc.Subgraph[1];
	SCSubgraphRef BidPOC = sc.Subgraph[2];
	
	SCInputRef level_3      = sc.Input[0];
	SCInputRef colorAsk_3      = sc.Input[1];
	SCInputRef colorBid_3      = sc.Input[2];
	SCInputRef level_2      = sc.Input[3];
	SCInputRef colorAsk_2      = sc.Input[4];
	SCInputRef colorBid_2      = sc.Input[5];
	SCInputRef level_1      = sc.Input[6];
	SCInputRef colorAsk_1   = sc.Input[7];
	SCInputRef colorBid_1      = sc.Input[8];
	SCInputRef barColorMode      = sc.Input[9];
	SCInputRef useBaseColor      = sc.Input[10];
	SCInputRef baseColor      = sc.Input[11];
	SCInputRef displayPOCs	= sc.Input[12];
	SCInputRef extendPOCs	= sc.Input[13];
	
	
	if (sc.SetDefaults)
	{
		// Set the configuration and defaults
		sc.GraphName = "Color Bars per Delta %";
		sc.StudyDescription = "";
		sc.AutoLoop = 1;  // true
		sc.GraphRegion = 0;
		
		sc.MaintainVolumeAtPriceData = 1;

		ColorDeltaBar.Name = "Color";
		ColorDeltaBar.DrawStyle = DRAWSTYLE_COLOR_BAR; 
		ColorDeltaBar.DrawZeros = false;
		
		AskPOC.Name = "Ask Side POCs";
		AskPOC.DrawStyle = DRAWSTYLE_DASH; 
		AskPOC.DrawZeros = false;
		AskPOC.PrimaryColor = RGB(255,255,0);
		AskPOC.LineWidth = 3;
		
		BidPOC.Name = "Bid Side POCs";
		BidPOC.DrawStyle = DRAWSTYLE_DASH; 
		BidPOC.DrawZeros = false;
		BidPOC.PrimaryColor = RGB(255,0, 255);
		BidPOC.LineWidth = 3;
		
		level_3.Name = " % Threshold 3";
		level_3.SetInt(24);
		colorAsk_3.Name = "Ask Delta 3";
		colorAsk_3.SetColor(140, 255, 140);
		colorBid_3.Name = "Bid Delta 3";
		colorBid_3.SetColor(255, 140, 140);
		
		level_2.Name = " % Threshold 2";
		level_2.SetInt(16);
		colorAsk_2.Name = "Ask Delta 2";
		colorAsk_2.SetColor(0, 255, 0);
		colorBid_2.Name = "Bid Delta 2";
		colorBid_2.SetColor(255, 0, 0);
		
		level_1.Name = " % Threshold 1";
		level_1.SetInt(8);
		colorAsk_1.Name = "Ask Delta 1";
		colorAsk_1.SetColor(0, 170, 0);
		colorBid_1.Name = "Bid Delta 2";
		colorBid_1.SetColor(170, 0, 0);
		
		barColorMode.Name = "Bar Color Mode";
		barColorMode.SetCustomInputStrings("Use thresholds; Use gradient: Base-Threshold 3");
		barColorMode.SetCustomInputIndex(1);
		
		useBaseColor.Name = "Color candles below threshold 1";
		useBaseColor.SetYesNo(0);
		
		baseColor.Name = "Base Color (for gradients or candles below thres. 1)";
		baseColor.SetColor(170, 170, 170);
		
		displayPOCs.Name = "Display Thresholds 2-3 bar-POCs";
		displayPOCs.SetYesNo(1);
		
		extendPOCs.Name = "Extend bar POCs";
		extendPOCs.SetYesNo(1);
		
		return;
	}
	
	ColorDeltaBar.Name = "Color";
	ColorDeltaBar.DrawStyle = DRAWSTYLE_COLOR_BAR; 
	
	float deltaPerc = 100 * ((float)sc.AskVolume[sc.Index] - (float)sc.BidVolume[sc.Index]) / (float)sc.Volume[sc.Index];
	
	//if using threshold colors
	if(barColorMode.GetIndex() == 0)
	{
		//at the beginning color bar with base color or don't color
		ColorDeltaBar.DataColor[sc.Index] = useBaseColor.GetYesNo() ? baseColor.GetColor() : 0;
		
		//sepecify color depending on delta threshold
		if(deltaPerc > level_3.GetInt() ) ColorDeltaBar.DataColor[sc.Index] = colorAsk_3.GetColor();
			else if(deltaPerc > level_2.GetInt() ) ColorDeltaBar.DataColor[sc.Index] = colorAsk_2.GetColor();
				else if(deltaPerc > level_1.GetInt() ) ColorDeltaBar.DataColor[sc.Index] = colorAsk_1.GetColor();
					else if(deltaPerc < -level_3.GetInt() ) ColorDeltaBar.DataColor[sc.Index] = colorBid_3.GetColor();
						else if(deltaPerc < -level_2.GetInt() ) ColorDeltaBar.DataColor[sc.Index] = colorBid_2.GetColor();
							else if(deltaPerc < -level_1.GetInt() ) ColorDeltaBar.DataColor[sc.Index] = colorBid_1.GetColor(); 
		
		//activate bar coloring if .DataColor contains any color
		ColorDeltaBar[sc.Index] = ColorDeltaBar.DataColor[sc.Index] != 0 ? 1 : 0;
	}
	else	//using gradient
	{
		ColorDeltaBar[sc.Index] = 1;	//always color candles
		float interpPerc = deltaPerc / level_3.GetInt();	//deltaPercent threshold 3 ratio
		if(deltaPerc > 0)		//set the candle color
			ColorDeltaBar.DataColor[sc.Index] = sc.RGBInterpolate(baseColor.GetColor(), colorAsk_3.GetColor(), min(interpPerc, 1) );
		else
			ColorDeltaBar.DataColor[sc.Index] = sc.RGBInterpolate(baseColor.GetColor(), colorBid_3.GetColor(), -max(interpPerc, -1) );
	}
	
	
	//si la barra ha cerrado, calulo POC de cada lado
	if(sc.GetBarHasClosedStatus(sc.Index) == BHCS_BAR_HAS_CLOSED)
	{
		if (deltaPerc > level_2.GetInt() )
		{
			const s_VolumeAtPriceV2 *p_VolumeAtPrice=NULL;
			int VAPSizeAtBarIndex = sc.VolumeAtPriceForBars->GetSizeAtBarIndex(sc.Index);
			unsigned int maxAskVol = 0;
			float maxAskPrice = 0;
			for (int VAPIndex = 0; VAPIndex < VAPSizeAtBarIndex; VAPIndex++)
			{
				if (!sc.VolumeAtPriceForBars->GetVAPElementAtIndex(sc.Index, VAPIndex, &p_VolumeAtPrice))
					break;

				if(p_VolumeAtPrice->AskVolume > maxAskVol)
				{
					maxAskVol = p_VolumeAtPrice->AskVolume;
					maxAskPrice = p_VolumeAtPrice->PriceInTicks * sc.TickSize;
				}
			}	
		
			if(displayPOCs.GetYesNo() ) AskPOC[sc.Index] = maxAskPrice;
			if(extendPOCs.GetYesNo() ) sc.AddLineUntilFutureIntersection(sc.Index, 0, maxAskPrice, AskPOC.PrimaryColor, 1, LINESTYLE_DOT, 0, 0, "");
		}
	
		if (deltaPerc < -level_2.GetInt() )
		{
			const s_VolumeAtPriceV2 *p_VolumeAtPrice=NULL;
			int VAPSizeAtBarIndex = sc.VolumeAtPriceForBars->GetSizeAtBarIndex(sc.Index);
			unsigned int maxBidVol = 0;
			float maxBidPrice = 0;
			for (int VAPIndex = 0; VAPIndex < VAPSizeAtBarIndex; VAPIndex++)
			{
				if (!sc.VolumeAtPriceForBars->GetVAPElementAtIndex(sc.Index, VAPIndex, &p_VolumeAtPrice))
					break;

				if(p_VolumeAtPrice->BidVolume > maxBidVol)
				{
					maxBidVol = p_VolumeAtPrice->BidVolume;
					maxBidPrice = p_VolumeAtPrice->PriceInTicks * sc.TickSize;
				}
			}	
		
			if(displayPOCs.GetYesNo() ) BidPOC[sc.Index] = maxBidPrice;
			if(extendPOCs.GetYesNo() ) sc.AddLineUntilFutureIntersection(sc.Index, 0, maxBidPrice, BidPOC.PrimaryColor, 1, LINESTYLE_DOT, 0, 0, "");
		}
	}
}


SCSFExport scsf_GapBars(SCStudyInterfaceRef sc)
{
	SCSubgraphRef GapUpBar = sc.Subgraph[0];
	SCSubgraphRef GapDownBar = sc.Subgraph[1];
	SCSubgraphRef NearestGapBelowBegin = sc.Subgraph[2];
	SCSubgraphRef NearestGapBelowEnd = sc.Subgraph[3];
	SCSubgraphRef NearestGapAboveBegin = sc.Subgraph[4];
	SCSubgraphRef NearestGapAboveEnd = sc.Subgraph[5];
	
	
	if (sc.SetDefaults)
	{
		// Set the configuration and defaults		
		sc.GraphName = "Gap Bars";
		sc.StudyDescription = "Gap bars as per Al Brooks";
		sc.AutoLoop = 1;  // true
		sc.GraphRegion = 0;
		
		GapUpBar.Name = "GapUp Bar Color and Range Fill";
		GapUpBar.DrawStyle = DRAWSTYLE_COLOR_BAR;
		GapUpBar.PrimaryColor = RGB(60,179,113);
		GapUpBar.SecondaryColor = RGB(190,255,125);
		GapUpBar.SecondaryColorUsed = 1;		
		GapUpBar.DrawZeros = false;
		
		GapDownBar.Name = "GapDown Bar Color and Range Fill";
		GapDownBar.DrawStyle = DRAWSTYLE_COLOR_BAR;
		GapDownBar.PrimaryColor = RGB(255,60,25);
		GapDownBar.SecondaryColor = RGB(255,160,145);
		GapDownBar.SecondaryColorUsed = 1;
		GapDownBar.DrawZeros = false;
		
		NearestGapBelowBegin.Name = "Nearest Gap Below Begin";
		NearestGapBelowBegin.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_TOP;
		NearestGapBelowBegin.PrimaryColor = RGB(60,179,113);
		//NearestGapBelowBegin.
		NearestGapBelowBegin.DrawZeros = false;
		
		NearestGapBelowEnd.Name = "Nearest Gap Below End";
		NearestGapBelowEnd.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_BOTTOM;
		NearestGapBelowEnd.PrimaryColor = RGB(60,179,113);
		//NearestGapBelowBegin.
		NearestGapBelowEnd.DrawZeros = false;
		
		NearestGapAboveBegin.Name = "Nearest Gap Above Begin";
		NearestGapAboveBegin.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_TOP;
		NearestGapAboveBegin.PrimaryColor = RGB(255,60,25);
		//NearestGapAboveBegin.
		NearestGapAboveBegin.DrawZeros = false;
		
		NearestGapAboveEnd.Name = "Nearest Gap Above End";
		NearestGapAboveEnd.DrawStyle = DRAWSTYLE_TRANSPARENT_FILL_RECTANGLE_BOTTOM;
		NearestGapAboveEnd.PrimaryColor = RGB(255,60,25);
		//NearestGapAboveEnd.
		NearestGapAboveEnd.DrawZeros = false;
		
		//persistent index for first rectangle ID
		sc.GetPersistentInt(2) = 5;
		
		return;
	}

	if(!sc.HideStudy)
	{
		//locked defaults
		GapUpBar.DrawStyle = DRAWSTYLE_COLOR_BAR;
		GapDownBar.DrawStyle = DRAWSTYLE_COLOR_BAR;
		
		//persistent variables
		int& alreadyGappingUp = sc.GetPersistentInt(0);
		int& alreadyGappingDown = sc.GetPersistentInt(1);
		//untested rectangles IDs 
		int& nextID = sc.GetPersistentInt(2);
		
		//full recalculation
		if (sc.IsFullRecalculation && sc.Index == 0)//This indicates a study is being recalculated.
		{
			// When there is a full recalculation of the study,
			// reset the persistent variables we are using
			alreadyGappingUp = 0;
			alreadyGappingDown = 0;
			nextID = 5;
		}
		

		//**** BEGIN
		//copio valor anterior de nearest gaps
		if(alreadyGappingDown == 0) {NearestGapAboveBegin[sc.Index] = NearestGapAboveBegin[sc.Index - 1]; NearestGapAboveEnd[sc.Index] = NearestGapAboveEnd[sc.Index - 1];}
		if(alreadyGappingUp == 0) {NearestGapBelowBegin[sc.Index] = NearestGapBelowBegin[sc.Index - 1]; NearestGapBelowEnd[sc.Index] = NearestGapBelowEnd[sc.Index - 1];}
		
		//si en este tick testamos los nearest gaps, recorro los untested gaps buscando cuál y los más cercanos por arriba y por abajo
		if( (sc.High[sc.Index] > NearestGapAboveBegin[sc.Index] && sc.Low[sc.Index] < NearestGapAboveBegin[sc.Index]) || (sc.High[sc.Index] > NearestGapBelowBegin[sc.Index] && sc.Low[sc.Index] < NearestGapBelowBegin[sc.Index]) )
		{
			//seteo a cero los nearest
			NearestGapAboveBegin[sc.Index] = 0; NearestGapAboveEnd[sc.Index] = 0;
			NearestGapBelowBegin[sc.Index] = 0; NearestGapBelowEnd[sc.Index] = 0;
			
			//recorro los untested gaps
			for(int x = nextID - 1; x >= 5; x--)
			{
				s_UseTool Rect;
				Rect.Clear();
				bool getLine = sc.GetACSDrawingByLineNumber(0, sc.GetPersistentInt(x), Rect);
				if(getLine)
				{	
					double begin = Rect.BeginValue, end = Rect.EndValue;
					//si este gap ha sido el que se ha testado, modifico el dibujo
					if(sc.High[sc.Index] > begin && sc.Low[sc.Index] < begin)
					{	
						//modifico el rectángulo para que termine en esta barra
						Rect.DrawingType = DRAWING_RECTANGLEHIGHLIGHT;
						Rect.EndIndex = sc.Index;
						Rect.AddMethod = UTAM_ADD_OR_ADJUST;				
						sc.UseTool(Rect);
						
						//y borro su ID de la array de untested
						for(int z = x; z < nextID; z++)
						{
							sc.GetPersistentInt(z) = sc.GetPersistentInt(z + 1);
						}
						nextID--;
						//continúo el loop y paso al siguiente untested gap
						continue;
					}
					
					//busco el primer gap por arriba y por abajo
					if(NearestGapAboveBegin[sc.Index] == 0 && begin > sc.Close[sc.Index])
					{
						NearestGapAboveBegin[sc.Index] = begin;
						NearestGapAboveEnd[sc.Index] = end;
					}				
					if(NearestGapBelowBegin[sc.Index] == 0 && begin < sc.Close[sc.Index])
					{
						NearestGapBelowBegin[sc.Index] = begin;
						NearestGapBelowEnd[sc.Index] = end;
					}
				}
				
				//si tengo gap por arriba y por abajo, rompo el loop
				if(NearestGapAboveBegin[sc.Index] != 0 && NearestGapBelowBegin[sc.Index] != 0) break;
			}
		}
				
		
		//*** SI LA BARRA HA CERRADO, BUSCO SI HA HABIDO /CONTINUADO GAPS
		if(sc.GetBarHasClosedStatus(sc.Index) == BHCS_BAR_HAS_CLOSED)
		{
			//**** VEO SI HAY GAPUP
			if(sc.Low[sc.Index] > sc.High[sc.Index - 2] && sc.Close[sc.Index] > sc.Close[sc.Index - 1])
			{
				//coloreo la barra/candle
				GapUpBar[sc.Index-1] = 1;
				
				//defino una variable  s_UseTool
				s_UseTool Rect;
				Rect.Clear();
				
				//compruebo si ya gappeé en la barra anterior o es gap nuevo
				//si es gap nuevo, creo el nivel
				if(alreadyGappingUp == 0)
				{				
					Rect.DrawingType = DRAWING_RECTANGLE_EXT_HIGHLIGHT;
					Rect.BeginIndex = sc.Index - 2;
					Rect.EndIndex = sc.Index + 1;
					Rect.BeginValue = sc.High[sc.Index - 2];
					Rect.EndValue = sc.Low[sc.Index];
					Rect.Color = GapUpBar.PrimaryColor;
					Rect.SecondaryColor = GapUpBar.SecondaryColor;
					Rect.TransparencyLevel = sc.TransparencyLevel;
					Rect.NoVerticalOutline = 1;
					
					sc.UseTool(Rect);
					
					//y seteo alreaadyGappingUp con la ID del rectángulo y guardo la ID en untested y en nearestGap
					alreadyGappingUp = Rect.LineNumber;
					sc.GetPersistentInt(nextID) = alreadyGappingUp;
					NearestGapBelowBegin[sc.Index + 1] = Rect.BeginValue;
					NearestGapBelowEnd[sc.Index + 1] = Rect.EndValue;
					nextID++;
				}
				//si el gap es continuación, lo amplío
				else
				{
					bool getGap = sc.GetACSDrawingByLineNumber(0, alreadyGappingUp, Rect);
					if(getGap)
					{
						Rect.EndValue = sc.Low[sc.Index];
						Rect.EndIndex = sc.Index + 1;
						Rect.AddMethod = UTAM_ADD_OR_ADJUST;
						
						sc.UseTool(Rect);
						
						//modifico nearest gap
						NearestGapBelowBegin[sc.Index + 1] = Rect.BeginValue;
						NearestGapBelowEnd[sc.Index + 1] = Rect.EndValue;
						NearestGapBelowBegin[sc.Index] = 0;
						NearestGapBelowEnd[sc.Index] = 0;
					}
				}	
			}
			//si no hay gapUp, seteo a cero alreadyGappingUp
			else
			{
				alreadyGappingUp = 0;			
			}
			
			//**** VEO SI HAY GAPDOWN
			if(sc.High[sc.Index] < sc.Low[sc.Index - 2] && sc.Close[sc.Index] < sc.Close[sc.Index - 1])
			{
				//coloreo la barra/candle
				GapDownBar[sc.Index-1] = 1;
				
				//defino una variable  s_UseTool
				s_UseTool Rect;
				Rect.Clear();
				
				//compruebo si ya gappeé en la barra anterior o es gap nuevo
				//si es gap nuevo, creo el nivel
				if(alreadyGappingDown == 0)
				{				
					Rect.DrawingType = DRAWING_RECTANGLE_EXT_HIGHLIGHT;
					Rect.BeginIndex = sc.Index - 2;
					Rect.EndIndex = sc.Index + 1;
					Rect.BeginValue = sc.Low[sc.Index - 2];
					Rect.EndValue = sc.High[sc.Index];
					Rect.Color = GapDownBar.PrimaryColor;
					Rect.SecondaryColor = GapDownBar.SecondaryColor;
					Rect.TransparencyLevel = sc.TransparencyLevel;
					Rect.NoVerticalOutline = 1;
					
					sc.UseTool(Rect);
					
					//y seteo alreaadyGappingUp con la ID del rectángulo, guardo la ID en untested y en nearestGap
					alreadyGappingDown = Rect.LineNumber;
					sc.GetPersistentInt(nextID) = alreadyGappingDown;
					NearestGapAboveBegin[sc.Index + 1] = Rect.BeginValue;
					NearestGapAboveEnd[sc.Index + 1] = Rect.EndValue;
					nextID++;
				}
				//si el gap es continuación, lo amplío
				else
				{
					bool getGap = sc.GetACSDrawingByLineNumber(0, alreadyGappingDown, Rect);
					if(getGap)
					{
						Rect.EndValue = sc.High[sc.Index];
						Rect.EndIndex = sc.Index + 1;
						Rect.AddMethod = UTAM_ADD_OR_ADJUST;
						
						sc.UseTool(Rect);
						
						//modifico nearest gap
						NearestGapAboveBegin[sc.Index + 1] = Rect.BeginValue;
						NearestGapAboveEnd[sc.Index + 1] = Rect.EndValue;				
						NearestGapAboveBegin[sc.Index] = 0;
						NearestGapAboveEnd[sc.Index] = 0;
					}
				}	
			}
			//si no hay gapDown, seteo a cero alreadyGappingDown
			else
			{
				alreadyGappingDown = 0;			
			}
		}
	}
}


SCSFExport scsf_InitialBalanceSession(SCStudyInterfaceRef sc)
{
	SCSubgraphRef Subgraph_IBHExt6  = sc.Subgraph[0];
	SCSubgraphRef Subgraph_IBHExt5  = sc.Subgraph[1];
	SCSubgraphRef Subgraph_IBHExt4  = sc.Subgraph[2];
	SCSubgraphRef Subgraph_IBHExt3  = sc.Subgraph[3];
	SCSubgraphRef Subgraph_IBHExt2  = sc.Subgraph[4];
	SCSubgraphRef Subgraph_IBHExt1  = sc.Subgraph[5];
	SCSubgraphRef Subgraph_IBHigh   = sc.Subgraph[6];
	SCSubgraphRef Subgraph_IBMid    = sc.Subgraph[7];
	SCSubgraphRef Subgraph_IBLow    = sc.Subgraph[8];
	SCSubgraphRef Subgraph_IBLExt1  = sc.Subgraph[9];
	SCSubgraphRef Subgraph_IBLExt2  = sc.Subgraph[10];
	SCSubgraphRef Subgraph_IBLExt3  = sc.Subgraph[11];
	SCSubgraphRef Subgraph_IBLExt4  = sc.Subgraph[12];
	SCSubgraphRef Subgraph_IBLExt5  = sc.Subgraph[13];
	SCSubgraphRef Subgraph_IBLExt6  = sc.Subgraph[14];
	
	SCInputRef Input_IBType      = sc.Input[0];
	SCInputRef Input_PeriodEndAsMinutesFromSessionStart = sc.Input[1];
	SCInputRef Input_PeriodEndAsSecondsFromSessionStart = sc.Input[2];
	SCInputRef Input_RoundExt    = sc.Input[3];
	SCInputRef Input_NumberDaysToCalculate = sc.Input[4];

	SCInputRef Input_Multiplier1 = sc.Input[5];
	SCInputRef Input_Multiplier2 = sc.Input[6];
	SCInputRef Input_Multiplier3 = sc.Input[7];
	SCInputRef Input_Multiplier4 = sc.Input[8];
	SCInputRef Input_Multiplier5 = sc.Input[9];
	SCInputRef Input_Multiplier6 = sc.Input[10];

	if (sc.SetDefaults)
	{
		sc.GraphName		= "Session's Initial Balance";
		sc.DrawZeros		= 0;
		sc.GraphRegion		= 0;
		sc.AutoLoop			= 1;

		sc.ScaleRangeType = SCALE_SAMEASREGION;

		Subgraph_IBHExt6.Name = "IB High Ext 6";
		Subgraph_IBHExt6.PrimaryColor = RGB(0, 255, 0);
		Subgraph_IBHExt6.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBHExt6.DrawZeros = false;

		Subgraph_IBHExt5.Name = "IB High Ext 5";
		Subgraph_IBHExt5.PrimaryColor = RGB(0, 255, 0);
		Subgraph_IBHExt5.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBHExt5.DrawZeros = false;

		Subgraph_IBHExt4.Name = "IB High Ext 4";
		Subgraph_IBHExt4.PrimaryColor = RGB(0, 255, 0);
		Subgraph_IBHExt4.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBHExt4.DrawZeros = false;

		Subgraph_IBHExt3.Name = "IB High Ext 3";
		Subgraph_IBHExt3.PrimaryColor = RGB(0, 255, 0);
		Subgraph_IBHExt3.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBHExt3.DrawZeros = false;

		Subgraph_IBHExt2.Name = "IB High Ext 2";
		Subgraph_IBHExt2.PrimaryColor = RGB(0, 255, 0);
		Subgraph_IBHExt2.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBHExt2.DrawZeros = false;

		Subgraph_IBHExt1.Name = "IB High Ext 1";
		Subgraph_IBHExt1.PrimaryColor = RGB(0, 255, 0);
		Subgraph_IBHExt1.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBHExt1.DrawZeros = false;

		Subgraph_IBHigh.Name = "IB High";
		Subgraph_IBHigh.PrimaryColor = RGB(128, 255, 128);
		Subgraph_IBHigh.DrawStyle = DRAWSTYLE_DASH;
		Subgraph_IBHigh.DrawZeros = false;

		Subgraph_IBMid.Name = "IB Mid";
		Subgraph_IBMid.PrimaryColor = RGB(255, 255, 255);
		Subgraph_IBMid.DrawStyle = DRAWSTYLE_DASH;
		Subgraph_IBMid.DrawZeros = false;

		Subgraph_IBLow.Name = "IB Low";
		Subgraph_IBLow.PrimaryColor = RGB(255, 128, 128);
		Subgraph_IBLow.DrawStyle = DRAWSTYLE_DASH;
		Subgraph_IBLow.DrawZeros = false;

		Subgraph_IBLExt1.Name = "IB Low Ext 1";
		Subgraph_IBLExt1.PrimaryColor = RGB(255, 0, 0);
		Subgraph_IBLExt1.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBLExt1.DrawZeros = false;

		Subgraph_IBLExt2.Name = "IB Low Ext 2";
		Subgraph_IBLExt2.PrimaryColor = RGB(255, 0, 0);
		Subgraph_IBLExt2.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBLExt2.DrawZeros = false;

		Subgraph_IBLExt3.Name = "IB Low Ext 3";
		Subgraph_IBLExt3.PrimaryColor = RGB(255, 0, 0);
		Subgraph_IBLExt3.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBLExt3.DrawZeros = false;

		Subgraph_IBLExt4.Name = "IB Low Ext 4";
		Subgraph_IBLExt4.PrimaryColor = RGB(255, 0, 0);
		Subgraph_IBLExt4.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBLExt4.DrawZeros = false;

		Subgraph_IBLExt5.Name = "IB Low Ext 5";
		Subgraph_IBLExt5.PrimaryColor = RGB(255, 0, 0);
		Subgraph_IBLExt5.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBLExt5.DrawZeros = false;

		Subgraph_IBLExt6.Name = "IB Low Ext 6";
		Subgraph_IBLExt6.PrimaryColor = RGB(255, 0, 0);
		Subgraph_IBLExt6.DrawStyle = DRAWSTYLE_IGNORE;
		Subgraph_IBLExt6.DrawZeros = false;


		// Inputs
		Input_IBType.Name = "Initial Balance For Session";
		Input_IBType.SetCustomInputStrings("Day Session (RTH); Evening Session (ETH)");
		Input_IBType.SetCustomInputIndex(0);

		Input_PeriodEndAsMinutesFromSessionStart.Name = "Period End As Minutes from Session Start";
		Input_PeriodEndAsMinutesFromSessionStart.SetInt(30);
		
		Input_PeriodEndAsSecondsFromSessionStart.Name = "And/or Seconds (Opening Range)";
		Input_PeriodEndAsSecondsFromSessionStart.SetInt(0);
		
		Input_RoundExt.Name = "Round Extensions to TickSize";
		Input_RoundExt.SetYesNo(1);

		Input_NumberDaysToCalculate.Name = "Number of Days to Calculate";
		Input_NumberDaysToCalculate.SetInt(100);
		Input_NumberDaysToCalculate.SetIntLimits(1,INT_MAX);

		Input_Multiplier1.Name = "Extension Multiplier 1";
		Input_Multiplier1.SetFloat(.5f);
		Input_Multiplier2.Name = "Extension Multiplier 2";
		Input_Multiplier2.SetFloat(1.0f);
		Input_Multiplier3.Name = "Extension Multiplier 3";
		Input_Multiplier3.SetFloat(1.5f);
		Input_Multiplier4.Name = "Extension Multiplier 4";
		Input_Multiplier4.SetFloat(2.0f);
		Input_Multiplier5.Name = "Extension Multiplier 5";
		Input_Multiplier5.SetFloat(2.5f);
		Input_Multiplier6.Name = "Extension Multiplier 6";
		Input_Multiplier6.SetFloat(3.0f);

		return;
	}

	// Persist vars
	int& PeriodFirstIndex = sc.GetPersistentInt(1);
	
	SCDateTime& PeriodStartDateTime = sc.GetPersistentSCDateTime(1);
	SCDateTime& PeriodEndDateTime   = sc.GetPersistentSCDateTime(2);

	float& PeriodHigh       = sc.GetPersistentFloat(1);
	float& PeriodLow        = sc.GetPersistentFloat(2);
	float& PeriodMid        = sc.GetPersistentFloat(3);
	float& PeriodHighExt1   = sc.GetPersistentFloat(4);
	float& PeriodHighExt2   = sc.GetPersistentFloat(5);
	float& PeriodHighExt3   = sc.GetPersistentFloat(6);
	float& PeriodHighExt4   = sc.GetPersistentFloat(7);
	float& PeriodHighExt5   = sc.GetPersistentFloat(8);
	float& PeriodHighExt6   = sc.GetPersistentFloat(9);
	float& PeriodLowExt1    = sc.GetPersistentFloat(10);
	float& PeriodLowExt2    = sc.GetPersistentFloat(11);
	float& PeriodLowExt3    = sc.GetPersistentFloat(12);
	float& PeriodLowExt4    = sc.GetPersistentFloat(13);
	float& PeriodLowExt5    = sc.GetPersistentFloat(14);
	float& PeriodLowExt6    = sc.GetPersistentFloat(15);

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
	if (Input_IBType.GetIndex() == 0)
		StartDateTime.SetTime(sc.StartTime1);
	else
		StartDateTime.SetTime(sc.StartTime2);
		
	if (PrevBarDateTime < StartDateTime && CurrentBarDateTime >= StartDateTime)
	{
		PeriodFirstIndex = sc.Index;
		PeriodHigh = -FLT_MAX;
		PeriodLow  = FLT_MAX;

		PeriodStartDateTime = StartDateTime;
		
		//Set end of Initial Balance as minutes from session start
		PeriodEndDateTime = PeriodStartDateTime;
		PeriodEndDateTime += SCDateTime::SECONDS(Input_PeriodEndAsMinutesFromSessionStart.GetInt() * SECONDS_PER_MINUTE
			+ Input_PeriodEndAsSecondsFromSessionStart.GetInt() - 1);
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

			float Range = PeriodHigh - PeriodLow;

			PeriodHighExt1 = PeriodHigh + Input_Multiplier1.GetFloat() * Range; 
			PeriodHighExt2 = PeriodHigh + Input_Multiplier2.GetFloat() * Range; 
			PeriodHighExt3 = PeriodHigh + Input_Multiplier3.GetFloat() * Range; 
			PeriodHighExt4 = PeriodHigh + Input_Multiplier4.GetFloat() * Range; 
			PeriodHighExt5 = PeriodHigh + Input_Multiplier5.GetFloat() * Range; 
			PeriodHighExt6 = PeriodHigh + Input_Multiplier6.GetFloat() * Range; 

			PeriodLowExt1 = PeriodLow - Input_Multiplier1.GetFloat() * Range; 
			PeriodLowExt2 = PeriodLow - Input_Multiplier2.GetFloat() * Range; 
			PeriodLowExt3 = PeriodLow - Input_Multiplier3.GetFloat() * Range; 
			PeriodLowExt4 = PeriodLow - Input_Multiplier4.GetFloat() * Range; 
			PeriodLowExt5 = PeriodLow - Input_Multiplier5.GetFloat() * Range; 
			PeriodLowExt6 = PeriodLow - Input_Multiplier6.GetFloat() * Range; 

			if (Input_RoundExt.GetYesNo())
			{
				PeriodHighExt1 = sc.RoundToTickSize(PeriodHighExt1, sc.TickSize); 
				PeriodHighExt2 = sc.RoundToTickSize(PeriodHighExt2, sc.TickSize); 
				PeriodHighExt3 = sc.RoundToTickSize(PeriodHighExt3, sc.TickSize); 
				PeriodHighExt4 = sc.RoundToTickSize(PeriodHighExt4, sc.TickSize); 
				PeriodHighExt5 = sc.RoundToTickSize(PeriodHighExt5, sc.TickSize); 
				PeriodHighExt6 = sc.RoundToTickSize(PeriodHighExt6, sc.TickSize); 

				PeriodLowExt1 = sc.RoundToTickSize(PeriodLowExt1, sc.TickSize); 
				PeriodLowExt2 = sc.RoundToTickSize(PeriodLowExt2, sc.TickSize); 
				PeriodLowExt3 = sc.RoundToTickSize(PeriodLowExt3, sc.TickSize); 
				PeriodLowExt4 = sc.RoundToTickSize(PeriodLowExt4, sc.TickSize); 
				PeriodLowExt5 = sc.RoundToTickSize(PeriodLowExt5, sc.TickSize); 
				PeriodLowExt6 = sc.RoundToTickSize(PeriodLowExt6, sc.TickSize); 
			}

			for (int Index = PeriodFirstIndex; Index < sc.Index; Index++)
			{
				Subgraph_IBHigh[Index]  = PeriodHigh;
				Subgraph_IBLow[Index]   = PeriodLow;
				Subgraph_IBMid[Index]   = PeriodMid;
				Subgraph_IBHExt1[Index] = PeriodHighExt1;
				Subgraph_IBHExt2[Index] = PeriodHighExt2;
				Subgraph_IBHExt3[Index] = PeriodHighExt3;
				Subgraph_IBHExt4[Index] = PeriodHighExt4;
				Subgraph_IBHExt5[Index] = PeriodHighExt5;
				Subgraph_IBHExt6[Index] = PeriodHighExt6;
				Subgraph_IBLExt1[Index] = PeriodLowExt1;
				Subgraph_IBLExt2[Index] = PeriodLowExt2;
				Subgraph_IBLExt3[Index] = PeriodLowExt3;
				Subgraph_IBLExt4[Index] = PeriodLowExt4;
				Subgraph_IBLExt5[Index] = PeriodLowExt5;
				Subgraph_IBLExt6[Index] = PeriodLowExt6;			
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
		Subgraph_IBHExt1[sc.Index] = PeriodHighExt1;
		Subgraph_IBHExt2[sc.Index] = PeriodHighExt2;
		Subgraph_IBHExt3[sc.Index] = PeriodHighExt3;
		Subgraph_IBHExt4[sc.Index] = PeriodHighExt4;
		Subgraph_IBHExt5[sc.Index] = PeriodHighExt5;
		Subgraph_IBHExt6[sc.Index] = PeriodHighExt6;
		Subgraph_IBLExt1[sc.Index] = PeriodLowExt1;
		Subgraph_IBLExt2[sc.Index] = PeriodLowExt2;
		Subgraph_IBLExt3[sc.Index] = PeriodLowExt3;
		Subgraph_IBLExt4[sc.Index] = PeriodLowExt4;
		Subgraph_IBLExt5[sc.Index] = PeriodLowExt5;
		Subgraph_IBLExt6[sc.Index] = PeriodLowExt6;
	}
}




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

