// CanonAttr.cpp
// Lookup function to translate Canonical Attributes into
// human-readable names.
// Built from V1 AssignDLGAttrs from dlg.c on 3/28/96 by CXH
// Copyright 1996

#include "stdafx.h"
#include "Joe.h"

#define CANONNAMEBUFSIZE 256
static char CanonNameBuf[CANONNAMEBUFSIZE];

void CanonNameCat(const char *NewPart);

const char *Joe::CanonName(void)
{
if(TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return(NULL);
	} // if

return(DecodeCanonName(AttribInfo.CanonicalTypeMajor, AttribInfo.CanonicalTypeMinor));
} // Joe::CanonName

const char *Joe::SecondaryName(void)
{
if(TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return(NULL);
	} // if

return(DecodeCanonName(AttribInfo.SecondTypeMajor, AttribInfo.SecondTypeMinor));
} // Joe::SecondaryName

const char *DecodeCanonName(unsigned short Major, unsigned short Minor)
{
char str[80];

CanonNameBuf[0] = NULL;

 switch (Major)
  {
  case 20:
  	{
	switch(Minor)
		{
		case 0:
			{
			CanonNameCat("Photo-revised");
			break;
			} // 
		case 200:
		case 201:
		case 202:
		case 203:
			{
			CanonNameCat("Contour");
			break;
			} // 
		case 204:
			{
			CanonNameCat("Amended Contour");
			break;
			} // 
		case 205:
			{
			CanonNameCat("Bathymetric Contour");
			break;
			} // 
		case 206:
			{
			CanonNameCat("Depth Curve");
			break;
			} // 
		case 207:
			{
			CanonNameCat("Watershed divide");
			break;
			} // 
		case 208:
			{
			CanonNameCat("Closure Line");
			break;
			} // 
		case 300:
			{
			CanonNameCat("Spot Elevation (<3rd order)");
			break;
			} // 
		case 301:
			{
			CanonNameCat("Spot Elevation (<3rd order) not at ground level");
			break;
			} // 
		case 600:
		case 601:
		case 602:
		case 603:
		case 604:
		case 605:
		case 606:
		case 607:
		case 608:
		case 609:
			{
			CanonNameCat("Decimal fractions of feet or meters");
			break;
			} // 
		case 610:
			{
			CanonNameCat("Approximate");
			break;
			} // 
		case 611:
			{
			CanonNameCat("Depression");
			break;
			} // 
		case 612:
			{
			CanonNameCat("Glacier or snowfield");
			break;
			} // 
		case 613:
			{
			CanonNameCat("Underwater");
			break;
			} // 
		case 614:
			{
			CanonNameCat("Best estimate");
			break;
			} // 

		} // Minor
	break;
	} // Hypsography
  case 24:
  	{
	sprintf(str, "Elevation: %dm", Minor);
	CanonNameCat(str);
	break;
	} // Spot Height
  case 26:
  	{
	sprintf(str, "Spot Height: %dm", Minor);
	CanonNameCat(str);
	break;
	} // Spot Height
  case 29:
  	{
	sprintf(str, "Coincedent Feature: %dm", Minor);
	CanonNameCat(str);
	break;
	} //
  case 40:
   {
   CanonNameCat("Hydrography Area");
   break;
   } /* Water Bodies 1:2000000 */
  case 50:
   {
   if (Minor == 0)
    {
	CanonNameCat("Photo Revision");
    break;
    } /* photo-revision */
   if (Minor >= 200 && Minor < 300)
    {
    switch (Minor)
     {
     case 200:
      {
      CanonNameCat("Shore");
      break;
      } /* shoreline */
     case 201:
      {
      CanonNameCat("Man Made Shore");
      break;
      } /* shoreline */
     case 202:
      {
      CanonNameCat("Closure Line");
      break;
      } /* shoreline */
     case 203:
      {
      CanonNameCat("Indefinite Shore");
      break;
      } /* shoreline */
     case 204:
      {
      CanonNameCat("Apparent Limit");
      break;
      } /* shoreline */
     case 205:
      {
      CanonNameCat("Outline of a Carolina Bay");
      break;
      } /* shoreline */
     case 206:
      {
      CanonNameCat("Danger Curve");
      break;
      } /* shoreline */
     } /* switch */
    } /* if */
   else if (Minor >= 300 && Minor < 400)
    {
    switch (Minor)
     {
     case 300:
      {
      CanonNameCat("Spring");
      break;
      } /*  */
     case 301:
      {
      CanonNameCat("Non Flowing Well");
      break;
      } /*  */
     case 302:
      {
      CanonNameCat("Flowing Well");
      break;
      } /*  */
     case 303:
      {
      CanonNameCat("Riser");
      break;
      } /*  */
     case 304:
      {
      CanonNameCat("Geyser");
      break;
      } /*  */
     case 305:
      {
      CanonNameCat("Windmill");
      break;
      } /*  */
     case 306:
      {
      CanonNameCat("Cistern");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* else if */
   else if (Minor >= 400 && Minor < 500)
    {
    switch (Minor)
     {
     case 400:
      {
      CanonNameCat("Rapids");
      break;
      } /*  */
     case 401:
      {
      CanonNameCat("Falls");
      break;
      } /*  */
     case 402:
      {
      CanonNameCat("Gravel Pit or Water-filled Quarry");
      break;
      } /*  */
     case 403:
      {
      CanonNameCat("Gaging Station");
      break;
      } /*  */
     case 404:
      {
      CanonNameCat("Pumping Station");
      break;
      } /*  */
     case 405:
      {
      CanonNameCat("Water Intake");
      break;
      } /*  */
     case 406:
      {
      CanonNameCat("Dam or Weir");
      break;
      } /*  */
     case 407:
      {
      CanonNameCat("Canal Lock or Sluice Gate");
      break;
      } /*  */
     case 408:
      {
      CanonNameCat("Spillway");
      break;
      } /*  */
     case 409:
      {
      CanonNameCat("Gate");
      break;
      } /*  */
     case 410:
      {
      CanonNameCat("Rock");
      break;
      } /*  */
     case 411:
      {
      CanonNameCat("Crevasse");
      break;
      } /*  */
     case 412:
      {
      CanonNameCat("Stream");
      break;
      } /*  */
     case 413:
      {
      CanonNameCat("Braided Stream");
      break;
      } /*  */
     case 414:
      {
      CanonNameCat("Ditch or Canal");
      break;
      } /*  */
     case 415:
      {
      CanonNameCat("Aqueduct");
      break;
      } /*  */
     case 416:
      {
      CanonNameCat("Flume");
      break;
      } /*  */
     case 417:
      {
      CanonNameCat("Penstock");
      break;
      } /*  */
     case 418:
      {
      CanonNameCat("Siphon");
      break;
      } /*  */
     case 419:
      {
      CanonNameCat("Channel in water area");
      break;
      } /*  */
     case 420:
      {
      CanonNameCat("Wash or Ephemeral Drain");
      break;
      } /*  */
     case 421:
      {
      CanonNameCat("Lake or Pond");
      break;
      } /*  */
     case 422:
      {
      CanonNameCat("Coral Reef");
      break;
      } /*  */
     case 423:
      {
      CanonNameCat("Sand in open water");
      break;
      } /*  */
     case 424:
      {
      CanonNameCat("Spoil area");
      break;
      } /*  */
     case 425:
      {
      CanonNameCat("Fish ladders");
      break;
      } /*  */
     case 426:
      {
      CanonNameCat("Holiday area");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* else if */
   else if (Minor >= 500)
    {
    switch (Minor)
     {
     case 601:
      {
      CanonNameCat("Underground");
      break;
      } /*  */
     case 602:
      {
      CanonNameCat("Overpassing");
      break;
      } /*  */
     case 603:
      {
      CanonNameCat("Elevated");
      break;
      } /*  */
     case 604:
      {
      CanonNameCat("Tunnel");
      break;
      } /*  */
     case 605:
      {
      CanonNameCat("Right Bank");
      break;
      } /*  */
     case 606:
      {
      CanonNameCat("Left Bank");
      break;
      } /*  */
     case 607:
      {
      CanonNameCat("Under Construction");
      break;
      } /*  */
     case 608:
      {
      CanonNameCat("Salt");
      break;
      } /*  */
     case 609:
      {
      CanonNameCat("Unsurveyed");
      break;
      } /*  */
     case 610:
      {
      CanonNameCat("Intermittant");
      break;
      } /*  */
     case 611:
      {
      CanonNameCat("Abandoned or discontinued");
      break;
      } /*  */
     case 612:
      {
      CanonNameCat("Submerged or sunken");
      break;
      } /*  */
     case 613:
      {
      CanonNameCat("Wooded");
      break;
      } /*  */
     case 614:
      {
      CanonNameCat("Dry");
      break;
      } /*  */
     case 615:
      {
      CanonNameCat("Mineral or Hot");
      break;
      } /*  */
     case 616:
      {
      CanonNameCat("Navigable, transportation");
      break;
      } /*  */
     case 617:
      {
      CanonNameCat("Underpassing");
      break;
      } /*  */
     case 618:
      {
      CanonNameCat("Earthen construction");
      break;
      } /*  */
     case 619:
      {
      CanonNameCat("Interpolated elevation");
      break;
      } /*  */
     case 621:
     case 622:
     case 623:
     case 624:
     case 625:
     case 626:
     case 627:
     case 628:
	 case 629:
      {
      CanonNameCat("Decimal feet or meters");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* else if */
   break;
   } /* hydrography */
  case 90:
   {
    switch (Minor)
     {
     case 100:
	  {
	  CanonNameCat("Civil Township/District");
	  break;
	  } // 
     case 101:
	  {
	  CanonNameCat("City/Village/Town");
	  break;
	  } // 
     case 103:
	  {
	  CanonNameCat("National Park");
	  break;
	  } // 
     case 104:
	  {
	  CanonNameCat("National Forest/Grassland");
	  break;
	  } // 
     case 105:
	  {
	  CanonNameCat("National Wildlife Refuge/Preserve/Hatchery");
	  break;
	  } // 
     case 106:
	  {
	  CanonNameCat("National Scenic Waterway/Wilderness");
	  break;
	  } // 
     case 107:
	  {
	  CanonNameCat("Indian Reservation");
	  break;
	  } // 
     case 108:
	  {
	  CanonNameCat("Military Reservation");
	  break;
	  } // 
     case 197:
	  {
	  CanonNameCat("Canada");
	  break;
	  } // 
     case 198:
	  {
	  CanonNameCat("Mexico");
	  break;
	  } // 
     case 199:
	  {
	  CanonNameCat("Outside Natl Boundary");
	  break;
	  } // 
	 } // Minor
   break;
   } // 
  case 91:
   {
   sprintf(str, "StateFIPS %d", Minor);
   CanonNameCat(str);
   break;
   } // State FIPS
  case 92:
   {
   sprintf(str, "CountyFIPS %d", Minor);
   CanonNameCat(str);
   break;
   } // County FIPS
  case 102:
   {
   sprintf(str, "I-%d", Minor);
   CanonNameCat(str);
   break;
   } /* Political or Administrative Boundaries 1:2000000 */
  case 103:
   {
   sprintf(str, "US-%d", Minor);
   CanonNameCat(str);
   break;
   } /* Political or Administrative Boundaries 1:2000000 */
  case 104:
   {
   sprintf(str, "ST-%d", Minor);
   CanonNameCat(str);
   break;
   } /* Political or Administrative Boundaries 1:2000000 */
  case 170:
   {
   if (Minor == 0)
    {
   	CanonNameCat("Photo Revision");
    break;
    } /* photo-revision */
   if (Minor >= 200 && Minor < 300)
    { // Transportation, roads, trails
    switch (Minor)
     {
     case 201:
      {
      CanonNameCat("Class 1 Undivided");
      break;
      } /*  */
     case 202:
      {
      CanonNameCat("Class 1 Center Line");
      break;
      } /*  */
     case 203:
      {
      CanonNameCat("Class 1 Divided, lanes seperated");
      break;
      } /*  */
     case 204:
      {
      CanonNameCat("Class 1 One-Way");
      break;
      } /*  */
     case 205:
      {
      CanonNameCat("Class 2 Undivided Road");
      break;
      } /*  */
     case 206:
      {
      CanonNameCat("Class 2 CenterLine");
      break;
      } /*  */
     case 207:
      {
      CanonNameCat("Class 2 Divided Road");
      break;
      } /*  */
     case 208:
      {
      CanonNameCat("Class 2 One-Way Road");
      break;
      } /*  */
     case 209:
      {
      CanonNameCat("Class 3 Road");
      break;
      } /*  */
     case 210:
      {
      CanonNameCat("Class 4 Road");
      break;
      } /*  */
     case 211:
      {
      CanonNameCat("Class 5 Trail");
      break;
      } /*  */
     case 212:
      {
      CanonNameCat("Class 5 Four Wheel Drive");
      break;
      } /*  */
     case 213:
      {
      CanonNameCat("Foot Bridge");
      break;
      } /*  */
     case 214:
      {
      CanonNameCat("Ferry Crossing");
      break;
      } /*  */
     case 215:
      {
      CanonNameCat("Parking Area");
      break;
      } /*  */
     case 216:
      {
      CanonNameCat("Arbitrary Extention");
      break;
      } /*  */
     case 217:
      {
      CanonNameCat("Class 3 CenterLine");
      break;
      } /*  */
     case 218:
      {
      CanonNameCat("Class 3 Divided Road");
      break;
      } /*  */
     case 221:
      {
      CanonNameCat("Class 3 Road or Street, One Way");
      break;
      } /*  */
     case 222:
      {
      CanonNameCat("Road in Transition");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* if */
   else if (Minor >= 400 && Minor < 500)
    {
    switch (Minor)
     {
     case 401:
      {
      CanonNameCat("Traffic Circle");
      break;
      } /*  */
     case 402:
      {
      CanonNameCat("CloverLeaf or Interchange");
      break;
      } /*  */
     case 403:
      {
      CanonNameCat("TollGate");
      break;
      } /*  */
     case 404:
      {
      CanonNameCat("Weigh Station");
      break;
      } /*  */
     case 405:
      {
      CanonNameCat("Nonstandard Section");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* else if */
   else if (Minor >= 500)
    {
    switch (Minor)
     {
     case 600:
      {
      CanonNameCat("Historical");
      break;
      } /*  */
     case 601:
      {
      CanonNameCat("In Tunnel");
      break;
      } /*  */
     case 602:
      {
      CanonNameCat("Overpassing on Bridge");
      break;
      } /*  */
     case 603:
      {
      CanonNameCat("Under Construction, Class unknown");
      break;
      } /*  */
     case 604:
      {
      CanonNameCat("Under Construction");
      break;
      } /*  */
     case 605:
      {
      CanonNameCat("Labelled Old Railroad Grade");
      break;
      } /*  */
     case 606:
      {
      CanonNameCat("Submerged or in ford");
      break;
      } /*  */
     case 607:
      {
      CanonNameCat("Underpassing");
      break;
      } /*  */
     case 608:
      {
      CanonNameCat("Limited Access");
      break;
      } /*  */
     case 609:
      {
      CanonNameCat("TollRoad");
      break;
      } /*  */
     case 610:
      {
      CanonNameCat("Private or Controlled Access");
      break;
      } /*  */
     case 611:
      {
      CanonNameCat("Proposed");
      break;
      } /*  */
     case 612:
      {
      CanonNameCat("Double Deck");
      break;
      } /*  */
     case 613:
      {
      CanonNameCat("In Service Facility or Rest Area");
      break;
      } /*  */
     case 614:
      {
      CanonNameCat("Elevated");
      break;
      } /*  */
     case 615:
      {
      CanonNameCat("Bypass Route");
      break;
      } /*  */
     case 616:
      {
      CanonNameCat("Alternate Route");
      break;
      } /*  */
     case 617:
      {
      CanonNameCat("Business Route");
      break;
      } /*  */
     case 618:
      {
      CanonNameCat("Drawbridge");
      break;
      } /*  */
     case 619:
      {
      CanonNameCat("Spur");
      break;
      } /*  */
     case 620:
      {
      CanonNameCat("Loop");
      break;
      } /*  */
     case 621:
      {
      CanonNameCat("Connector");
      break;
      } /*  */
     case 622:
      {
      CanonNameCat("Truck Route");
      break;
      } /*  */
     case 650:
      {
      CanonNameCat("Width: 46'-55'");
      break;
      } /*  */
     case 651:
      {
      CanonNameCat("Width: 56'-65'");
      break;
      } /*  */
     case 652:
      {
      CanonNameCat("Width: 66'-75'");
      break;
      } /*  */
     case 653:
      {
      CanonNameCat("Width: 76'-85'");
      break;
      } /*  */
     case 654:
      {
      CanonNameCat("Width: 86'-95'");
      break;
      } /*  */
     case 655:
      {
      CanonNameCat("Width: 96'-105'");
      break;
      } /*  */
     case 656:
      {
      CanonNameCat("Width: 106'-115'");
      break;
      } /*  */
     case 657:
      {
      CanonNameCat("Width: 116'-125'");
      break;
      } /*  */
     case 658:
      {
      CanonNameCat("Width: 126'-135'");
      break;
      } /*  */
     case 659:
      {
      CanonNameCat("Width: 136'-145'");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* else if */
   break;
   } /* transportation */
  case 171: // Minor = Num of lanes
   {
   sprintf(str, "%d-lane", Minor);
   CanonNameCat(str);
   break;
   } //
  case 172: // Minor = Interstate Rt Number
   {
   sprintf(str, "Interstate I-%d", Minor);
   CanonNameCat(str);
   break;
   } //
  case 173: // Minor = US Route number
   {
   sprintf(str, "US Route %d", Minor);
   CanonNameCat(str);
   break;
   } //
  case 174: // Minor = State Route Number
   {
   sprintf(str, "State Highway %d", Minor);
   CanonNameCat(str);
   break;
   } //
  case 175: // Minor = Reservation, park or Military Route Number
   {
   sprintf(str, "Road %d", Minor);
   CanonNameCat(str);
   break;
   } //
  case 176: // Minor = Country route number
   {
   sprintf(str, "Country Route %d", Minor);
   CanonNameCat(str);
   break;
   } //
  case 177: // Minor = xx/yy 00=blank 01=a, 26=z
   {
   //sprintf(str, "%d-lane", Minor);
   //CanonNameCat(str);
   break;
   } //
  case 180:
   { // Railroad
   if (Minor == 0)
    {
	CanonNameCat("Photo Revision");
    break;
    } /* photo-revision */
   if (Minor >= 200 && Minor < 300)
    {
    switch (Minor)
     {
     case 201:
      {
      CanonNameCat("Railroad");
      break;
      } /*  */
     case 202:
      {
      CanonNameCat("Railroad In Street");
      break;
      } /*  */
     case 204:
      {
      CanonNameCat("Carline");
      break;
      } /*  */
     case 205:
      {
      CanonNameCat("Cog Railroad/Incline RR/Logging tram");
      break;
      } /*  */
     case 207:
      {
      CanonNameCat("Ferry Cross");
      break;
      } /*  */
     case 208:
      {
      CanonNameCat("Railroad Siding");
      break;
      } /*  */
     case 209:
      {
      CanonNameCat("Yard Limit");
      break;
      } /*  */
     case 210:
      {
      CanonNameCat("Arbitrary Extension");
      break;
      } /*  */
     case 211:
      {
      CanonNameCat("Closure Line");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* if */
   else if (Minor >= 400 && Minor < 500)
    {
    switch (Minor)
     {
     case 400:
      {
      CanonNameCat("Railroad Station");
      break;
      } /*  */
     case 401:
      {
      CanonNameCat("Turntable");
      break;
      } /*  */
     case 402:
      {
      CanonNameCat("Roundhouse");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* else if */
   else if (Minor >= 500)
    {
    switch (Minor)
     {
     case 600:
      {
      CanonNameCat("Historical");
      break;
      } /*  */
     case 601:
      {
      CanonNameCat("In Tunnel");
      break;
      } /*  */
     case 602:
      {
      CanonNameCat("Overpassing on Bridge");
      break;
      } /*  */
     case 603:
      {
      CanonNameCat("Abandoned");
      break;
      } /*  */
     case 604:
      {
      CanonNameCat("Dismantled");
      break;
      } /*  */
     case 605:
      {
      CanonNameCat("Underpassing");
      break;
      } /*  */
     case 606:
      {
      CanonNameCat("Narrow Gauge");
      break;
      } /*  */
     case 607:
      {
      CanonNameCat("In Snowshed or under Structure");
      break;
      } /*  */
     case 608:
      {
      CanonNameCat("Under Construction");
      break;
      } /*  */
     case 609:
      {
      CanonNameCat("Elevated");
      break;
      } /*  */
     case 610:
      {
      CanonNameCat("Rapid Transit");
      break;
      } /*  */
     case 611:
      {
      CanonNameCat("Drawbridge");
      break;
      } /*  */
     case 612:
      {
      CanonNameCat("Private");
      break;
      } /*  */
     case 613:
      {
      CanonNameCat("US Government");
      break;
      } /*  */
     case 614:
      {
      CanonNameCat("Juxtaposition");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* else if */
   break;
   } /* railroad */
  case 190:
   {
   if (Minor == 0)
    {
	CanonNameCat("Photo Revision");
    break;
    } /* photo-revision */
   if (Minor >= 200 && Minor < 300)
    {
    switch (Minor)
     {
     case 201:
      {
      CanonNameCat("Pipeline");
      break;
      } /*  */
     case 202:
      {
      CanonNameCat("Power Transmission Line");
      break;
      } /*  */
     case 203:
      {
      CanonNameCat("Telephone or Telegraph");
      break;
      } /*  */
     case 204:
      {
      CanonNameCat("Aerial Tramway, Ski lift, Monorail");
      break;
      } /*  */
     case 205:
      {
      CanonNameCat("Arbitrary Line Extension");
      break;
      } /*  */
     case 206:
      {
      CanonNameCat("Line Closure");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* if */
   else if (Minor >= 300 && Minor < 400)
    {
    switch (Minor)
     {
     case 300:
      {
      CanonNameCat("Seaplane Anchorage");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* else if */
   else if (Minor >= 400 && Minor < 500)
    {
    switch (Minor)
     {
     case 400:
      {
      CanonNameCat("Power Station");
      break;
      } /*  */
     case 401:
      {
      CanonNameCat("Sub Station");
      break;
      } /*  */
     case 402:
      {
      CanonNameCat("Hydro-Electric Plant");
      break;
      } /*  */
     case 403:
      {
      CanonNameCat("Airport");
      break;
      } /*  */
     case 404:
      {
      CanonNameCat("Heliport");
      break;
      } /*  */
     case 405:
      {
      CanonNameCat("Launch Complex");
      break;
      } /*  */
     case 406:
      {
      CanonNameCat("Non-water Pump Station");
      break;
      } /*  */
     case 407:
      {
      CanonNameCat("Seaplane Ramp");
      break;
      } /*  */
     case 408:
      {
      CanonNameCat("Measuring Station");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* else if */
   else if (Minor >= 500)
    {
    switch (Minor)
     {
     case 600:
      {
      CanonNameCat("Underground");
      break;
      } /*  */
     case 601:
      {
      CanonNameCat("Under Construction");
      break;
      } /*  */
     case 602:
      {
      CanonNameCat("Abandoned");
      break;
      } /*  */
     case 603:
      {
      CanonNameCat("Above Ground");
      break;
      } /*  */
     case 604:
      {
      CanonNameCat("Labelled Closed");
      break;
      } /*  */
     case 605:
      {
      CanonNameCat("Unimproved, loose surface");
      break;
      } /*  */
     case 606:
      {
      CanonNameCat("Submerged");
      break;
      } /*  */
     case 607:
      {
      CanonNameCat("Nuclear");
      break;
      } /*  */
     } /* switch minor attribute code */
    } /* else if */
   break;
   } /* pipeline */
  case 290:
   {
   if (Minor == 0)
    {
	CanonNameCat("Photo Revision");
    break;
    } /* photo-revision */
   if (Minor == 2017)
    {
    CanonNameCat("Continental Divide");
    }
   else if (Minor >= 3000 && Minor < 5000)
    {
    if (Minor >= 3000 && Minor < 3070)
     {
     CanonNameCat("Stream");
     }
    else if (Minor >= 3070 && Minor < 3084)
     {
     CanonNameCat("Canal");
     }
    else if (Minor == 3086)
     {
     CanonNameCat("Perennial Ditch");
     }
    else if (Minor == 3095)
     {
     CanonNameCat("Intercoastal Waterway");
     }
    else if (Minor == 4000)
     {
     CanonNameCat("US Coastline");
     }
    else if (Minor >= 4000 && Minor < 4015)
     {
     CanonNameCat("Perennial Water Body");
     }
    else if (Minor >= 4021 && Minor < 4035)
     {
     CanonNameCat("Intermittant Water Body");
     }
    else if (Minor >= 4040 && Minor < 4046)
     {
     CanonNameCat("Marsh/Swamp and Salt Marsh");
     }
    else if (Minor >= 4050 && Minor < 4060)
     {
     CanonNameCat("Dry Lake or Alkali flat");
     }
    else if (Minor >= 4060 && Minor < 5000)
     {
     CanonNameCat("Glacier");
     }
    } /* if */
   else if (Minor >= 5000 && Minor < 5070)
    {
    CanonNameCat("Interstate");
    } /* if */
   else if (Minor >= 5070 && Minor < 6000)
    {
    CanonNameCat("Railroad");
    } /* if */
   else if (Minor >= 6000 && Minor < 7000)
    {
    CanonNameCat("Boundary");
    } /* if */
   else if (Minor >= 7000 && Minor < 8000)
    {
    CanonNameCat("Cultural");
    } /* if */
   else
    {
    CanonNameCat("Other");
    }
   break;
   } /* case 290 1:2000000 */
  default:
   {
   if(Major)
    {
    sprintf(str, "%d:%d ", Major, Minor);
    CanonNameCat(str);
	} // if
   } // default
  } /* switch major attribute code */

if(CanonNameBuf[0] != NULL)
	{
	return(CanonNameBuf);
	} // if
else
	{
	return(NULL);
	} // else

} // DecodeCanonName()

void CanonNameCat(const char *NewPart)
{
int Remaining, Extra;

Remaining =  (int)(CANONNAMEBUFSIZE - strlen(CanonNameBuf));
Extra = Remaining - (int)strlen(NewPart);

if(Remaining > 2)
	{
	if(CanonNameBuf[0] != NULL)
		{
		strcat(CanonNameBuf, ", ");
		} // if
	} // if
if(Extra > 2)
	{
	strcat(CanonNameBuf, NewPart);
	} // if
else
	{
	strncat(CanonNameBuf, NewPart, (unsigned)(Remaining - 3));
	CanonNameBuf[CANONNAMEBUFSIZE - 1] = NULL;
	} // else
} // CanonNameCat
