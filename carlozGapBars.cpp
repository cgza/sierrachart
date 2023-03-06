// The top of every source code file must include this line
#include "sierrachart.h"

// For reference, refer to this page:
// https://www.sierrachart.com/index.php?page=doc/AdvancedCustomStudyInterfaceAndLanguage.php

// This line is required. Change the text within the quote
// marks to what you want to name your group of custom studies. 
SCDLLName("carlozGapBars")

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