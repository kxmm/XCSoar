/*
Copyright_License {

  XCSoar Glide Computer - http://xcsoar.sourceforge.net/
  Copyright (C) 2000 - 2005

  	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@bigfoot.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

}
*/

#include "stdafx.h"

#include "externs.h"
#include "units.h"
#include "externs.h"
#include "Waypointparser.h"
#include "Logger.h"
#include "compatibility.h"
#ifdef OLDPPC
#include "XCSoarProcess.h"
#else
#include "Process.h"
#endif

extern BOOL extGPSCONNECT;

#include "WindowControls.h"
#include "dlgTools.h"

extern HWND   hWndMainWindow;
static WndForm *wf=NULL;
static bool multi_page = false;
static int status_page = 0;
static WndFrame *wStatus0=NULL;
static WndFrame *wStatus1=NULL;
static WndFrame *wStatus2=NULL;
static WndFrame *wStatus3=NULL;
static WndFrame *wStatus4=NULL;

#define NUMPAGES 5

static void NextPage(int Step){
  status_page += Step;
  if (status_page>=NUMPAGES) { status_page=0; }
  if (status_page<0) { status_page=NUMPAGES-1; }
  switch(status_page) {
  case 0:
    wf->SetCaption(gettext(TEXT("Status: Aircraft")));
    break;
  case 1:
    wf->SetCaption(gettext(TEXT("Status: System")));
    break;
  case 2:
    wf->SetCaption(gettext(TEXT("Status: Task")));
    break;
  case 3:
    wf->SetCaption(gettext(TEXT("Status: Rules")));
    break;
  case 4:
    wf->SetCaption(gettext(TEXT("Status: Times")));
    break;
  }
  wStatus0->SetVisible(status_page == 0);
  wStatus1->SetVisible(status_page == 1); 
  wStatus2->SetVisible(status_page == 2); 
  wStatus3->SetVisible(status_page == 3); 
  wStatus4->SetVisible(status_page == 4); 
}


static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}


static int FormKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){
	(void)lParam;
	(void)Sender;
  switch(wParam & 0xffff){
    case '6':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdPrev")))->GetHandle());
      NextPage(-1);
      //((WndButton *)wf->FindByName(TEXT("cmdPrev")))->SetFocused(true, NULL);
    return(0);
    case '7':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdNext")))->GetHandle());
      NextPage(+1);
      //((WndButton *)wf->FindByName(TEXT("cmdNext")))->SetFocused(true, NULL);
    return(0);
  }
  return(1);
}

static void OnNextClicked(WindowControl * Sender){
	(void)Sender;
  NextPage(+1);
}

static void OnPrevClicked(WindowControl * Sender){
	(void)Sender;
  NextPage(-1);
}


//////////////

static CallBackTableEntry_t CallBackTable[]={
  DeclearCallBackEntry(OnNextClicked),
  DeclearCallBackEntry(OnPrevClicked),
  DeclearCallBackEntry(NULL)
};

static bool first = true;

static void UpdateValuesSystem() {
  static int extGPSCONNECT_last = extGPSCONNECT;
  static int NAVWarning_last = GPS_INFO.NAVWarning;
  static int SatellitesUsed_last = GPS_INFO.SatellitesUsed;
  static int VarioAvailable_last = GPS_INFO.VarioAvailable;
  static int FLARM_Available_last = GPS_INFO.FLARM_Available;
  static bool LoggerActive_last = LoggerActive;
  static bool DeclaredToDevice_last = DeclaredToDevice;
  static double SupplyBatteryVoltage_last = GPS_INFO.SupplyBatteryVoltage;
  
  if (first ||
      (extGPSCONNECT_last != extGPSCONNECT) ||
      (NAVWarning_last != GPS_INFO.NAVWarning) ||
      (SatellitesUsed_last != GPS_INFO.SatellitesUsed) ||
      (VarioAvailable_last != GPS_INFO.VarioAvailable) ||
      (FLARM_Available_last != GPS_INFO.FLARM_Available) ||
      (LoggerActive_last != LoggerActive) ||
      (DeclaredToDevice_last != DeclaredToDevice) ||
      (SupplyBatteryVoltage_last != GPS_INFO.SupplyBatteryVoltage)) {
    first = false;

    extGPSCONNECT_last = extGPSCONNECT;
    NAVWarning_last = GPS_INFO.NAVWarning;
    SatellitesUsed_last = GPS_INFO.SatellitesUsed;
    VarioAvailable_last = GPS_INFO.VarioAvailable;
    FLARM_Available_last = GPS_INFO.FLARM_Available;
    LoggerActive_last = LoggerActive;
    DeclaredToDevice_last = DeclaredToDevice;
    SupplyBatteryVoltage_last = GPS_INFO.SupplyBatteryVoltage;

  } else {
    return;
  }
  
  TCHAR Temp[80];

  WndProperty* wp;
  wp = (WndProperty*)wf->FindByName(TEXT("prpGPS"));
  if (wp) {
    if (extGPSCONNECT) {
      if (GPS_INFO.NAVWarning) {
        wp->SetText(gettext(TEXT("Fix invalid")));
      } else {
        if (GPS_INFO.SatellitesUsed==0) {
          wp->SetText(gettext(TEXT("No fix")));
        } else {
          wp->SetText(gettext(TEXT("3D fix")));
        }
      }
      wp->RefreshDisplay();

      wp = (WndProperty*)wf->FindByName(TEXT("prpNumSat"));
      if (wp) {
        if (GPS_INFO.SatellitesUsed >= 0) {  // known numer of sats
          _stprintf(Temp,TEXT("%d"),GPS_INFO.SatellitesUsed);
        } else { // valid but unknown number of sats
          _stprintf(Temp,TEXT(">3"));
        }
        wp->SetText(Temp);
        wp->RefreshDisplay();
      }
    } else {
      wp->SetText(gettext(TEXT("Disconnected")));
      wp->RefreshDisplay();
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpVario"));
  if (wp) {
    if (GPS_INFO.VarioAvailable) {
      wp->SetText(gettext(TEXT("Connected")));
    } else {
      wp->SetText(gettext(TEXT("Disconnected")));
    }
    wp->RefreshDisplay();
  }

  if (wp) {
    wp = (WndProperty*)wf->FindByName(TEXT("prpFLARM"));
    if (GPS_INFO.FLARM_Available) {
      wp->SetText(gettext(TEXT("Connected")));
    } else {
      wp->SetText(gettext(TEXT("Disconnected")));
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLogger"));
  if (wp) {
    if (LoggerActive) {
      wp->SetText(gettext(TEXT("ON")));
    } else {
      wp->SetText(gettext(TEXT("OFF")));
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDeclared"));
  if (wp) {
    if (DeclaredToDevice) {
      wp->SetText(gettext(TEXT("YES")));
    } else {
      wp->SetText(gettext(TEXT("NO")));
    }
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBattery"));
  if (wp) {
    if (GPS_INFO.SupplyBatteryVoltage == 0) {
      _stprintf(Temp,TEXT("-"));
    } else {
      _stprintf(Temp,TEXT("%.1f V"),GPS_INFO.SupplyBatteryVoltage);
    }
    wp->SetText(Temp);
    wp->RefreshDisplay();
  }
}


static void UpdateValuesTimes(void) {
  WndProperty *wp;
  TCHAR Temp[1000];
  double sunsettime;
  int sunsethours;
  int sunsetmins;

  sunsettime = DoSunEphemeris(GPS_INFO.Longitude,
                              GPS_INFO.Latitude);
  sunsethours = (int)sunsettime;
  sunsetmins = (int)((sunsettime-sunsethours)*60);

  wp = (WndProperty*)wf->FindByName(TEXT("prpSunset"));
  if (wp) {
    _stprintf(Temp, TEXT("%02d:%02d"), sunsethours,sunsetmins);
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLocalTime"));
  if (wp) {
    Units::TimeToText(Temp, (int)DetectCurrentTime());
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTakeoffTime"));
  if (wp) {
    if (CALCULATED_INFO.FlightTime>0) {
      Units::TimeToText(Temp, 
                        (int)TimeLocal((long)CALCULATED_INFO.TakeOffTime));
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLandingTime"));
  if (wp) {
    if (!CALCULATED_INFO.Flying) {
      Units::TimeToText(Temp, 
                        (int)TimeLocal((long)(CALCULATED_INFO.TakeOffTime
                                              +CALCULATED_INFO.FlightTime)));
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFlightTime"));
  if (wp) {
    if (CALCULATED_INFO.FlightTime > 0){
      Units::TimeToText(Temp, (int)CALCULATED_INFO.FlightTime);
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }

}

static void UpdateValuesFlight(void) {
  WndProperty *wp;
  TCHAR Temp[1000];
  int iwaypoint= -1;
  double bearing;
  double distance;
  TCHAR sLongitude[16];
  TCHAR sLatitude[16];

  Units::LongitudeToString(GPS_INFO.Longitude, sLongitude, sizeof(sLongitude)-1);
  Units::LatitudeToString(GPS_INFO.Latitude, sLatitude, sizeof(sLatitude)-1);

  wp = (WndProperty*)wf->FindByName(TEXT("prpLongitude"));
  if (wp) {
    wp->SetText(sLongitude);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpLatitude"));
  if (wp) {
    wp->SetText(sLatitude);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpAltitude"));
  if (wp) {
    _stprintf(Temp, TEXT("%.0f %s"), 
              GPS_INFO.Altitude*ALTITUDEMODIFY, 
              Units::GetAltitudeName());
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpMaxHeightGain"));
  if (wp) {
    _stprintf(Temp, TEXT("%.0f %s"), 
              CALCULATED_INFO.MaxHeightGain*ALTITUDEMODIFY, 
              Units::GetAltitudeName());
    wp->SetText(Temp);
  }

  iwaypoint = FindNearestWayPoint(GPS_INFO.Longitude,
                                  GPS_INFO.Latitude,
                                  100000.0); // big range limit
  if (iwaypoint>=0) {

    DistanceBearing(GPS_INFO.Latitude,
                    GPS_INFO.Longitude,
                    WayPointList[iwaypoint].Latitude,
                    WayPointList[iwaypoint].Longitude,
                    &distance,
                    &bearing);

    wp = (WndProperty*)wf->FindByName(TEXT("prpNear"));
    if (wp) {
      wp->SetText(WayPointList[iwaypoint].Name);
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpBearing"));
    if (wp) {
      _stprintf(Temp, TEXT("%d�"), iround(bearing));
      wp->SetText(Temp);
    }

    wp = (WndProperty*)wf->FindByName(TEXT("prpDistance"));
    if (wp) {
      TCHAR DistanceText[MAX_PATH];
      Units::FormatUserDistance(distance,DistanceText, 10);
      wp->SetText(DistanceText);
    }

  } else {
    wp = (WndProperty*)wf->FindByName(TEXT("prpNear"));
    if (wp) {
      wp->SetText(TEXT("-"));
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpBearing"));
    if (wp) {
      wp->SetText(TEXT("-"));
    }
    wp = (WndProperty*)wf->FindByName(TEXT("prpDistance"));
    if (wp) {
      wp->SetText(TEXT("-"));
    }
  }

}


static void UpdateValuesRules(void) {
  WndProperty *wp;
  TCHAR Temp[80];

  wp = (WndProperty*)wf->FindByName(TEXT("prpValidStart"));
  if (wp) {
    if (CALCULATED_INFO.ValidStart) {
      wp->SetText(gettext(TEXT("TRUE")));
    } else {
      wp->SetText(gettext(TEXT("FALSE")));
    }
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpValidFinish"));
  if (wp) {
    if (CALCULATED_INFO.ValidFinish) {
      wp->SetText(gettext(TEXT("TRUE")));
    } else {
      wp->SetText(gettext(TEXT("FALSE")));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartTime"));
  if (wp) {
    if (CALCULATED_INFO.TaskStartTime>0) {
      Units::TimeToText(Temp, (int)TimeLocal((int)(CALCULATED_INFO.TaskStartTime)));
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartSpeed"));
  if (wp) {
    if (CALCULATED_INFO.TaskStartTime>0) {
      _stprintf(Temp, TEXT("%.0f %s"), 
                TASKSPEEDMODIFY*CALCULATED_INFO.TaskStartSpeed, 
                Units::GetTaskSpeedName());
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }
  // StartMaxHeight, StartMaxSpeed;

  //  double start_h;
  LockTaskData();

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartPoint"));

  if (ValidTaskPoint(0)) {
    //    start_h = WayPointList[Task[0].Index].Altitude;
    if (wp) {
      wp->SetText(WayPointList[Task[0].Index].Name);
    }
  } else {
    //    start_h = 0;
    if (wp) {
      wp->SetText(TEXT(""));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpStartHeight"));
  if (wp) {
    if (CALCULATED_INFO.TaskStartTime>0) {
      _stprintf(Temp, TEXT("%.0f %s"), 
                (CALCULATED_INFO.TaskStartAltitude)*ALTITUDEMODIFY, 
                Units::GetAltitudeName());
      wp->SetText(Temp);
    } else {
      wp->SetText(TEXT(""));
    }
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpFinishAlt"));
  if (wp) {
    double finish_min = FAIFinishHeight(&GPS_INFO, &CALCULATED_INFO, -1);
    _stprintf(Temp, TEXT("%.0f %s"), 
              finish_min*ALTITUDEMODIFY, 
              Units::GetAltitudeName());
    wp->SetText(Temp);
  }

  UnlockTaskData();
}


static void UpdateValuesTask(void) {
  WndProperty *wp;
  TCHAR Temp[80];

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskTime"));
  Units::TimeToText(Temp, (int)AATTaskLength*60);
  if (wp) {
    if (!AATEnabled) {
      wp->SetVisible(false);
    } else {
      wp->SetText(Temp);
    }
  }

  double dd = CALCULATED_INFO.TaskTimeToGo;
  if (CALCULATED_INFO.TaskStartTime>0.0) {
    dd += GPS_INFO.Time-CALCULATED_INFO.TaskStartTime;
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpETETime"));
  if (wp) {
    Units::TimeToText(Temp, (int)dd);
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRemainingTime"));
  if (wp) {
    Units::TimeToText(Temp, (int)CALCULATED_INFO.TaskTimeToGo);
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpTaskDistance"));
  if (wp) {
    _stprintf(Temp, TEXT("%.0f %s"), DISTANCEMODIFY*
              (CALCULATED_INFO.TaskDistanceToGo
               +CALCULATED_INFO.TaskDistanceCovered), 
              Units::GetDistanceName());
    wp->SetText(Temp);
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpRemainingDistance"));
  if (wp) {
    if (AATEnabled) {
      _stprintf(Temp, TEXT("%.0f %s"), 
                DISTANCEMODIFY*CALCULATED_INFO.AATTargetDistance, 
                Units::GetDistanceName());
    } else {
      _stprintf(Temp, TEXT("%.0f %s"), 
                DISTANCEMODIFY*CALCULATED_INFO.TaskDistanceToGo, 
                Units::GetDistanceName());
    }
    wp->SetText(Temp);
  }

  double d1 = 
    (CALCULATED_INFO.TaskDistanceToGo
     +CALCULATED_INFO.TaskDistanceCovered)/dd;
  // JMW TODO: this fails for OLC

  wp = (WndProperty*)wf->FindByName(TEXT("prpEstimatedSpeed"));
  if (wp) {
    _stprintf(Temp, TEXT("%.0f %s"), 
              TASKSPEEDMODIFY*d1, Units::GetTaskSpeedName());
    wp->SetText(Temp);
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpAverageSpeed"));
  if (wp) {
    _stprintf(Temp, TEXT("%.0f %s"), 
              TASKSPEEDMODIFY*CALCULATED_INFO.TaskSpeed, 
              Units::GetTaskSpeedName());
    wp->SetText(Temp);
  }
}


static int OnTimerNotify(WindowControl * Sender) {
  (void)Sender;

  UpdateValuesSystem();

  return 0;
}


//////////////

void dlgStatusShowModal(int start_page){

  if (start_page==-1) {
    multi_page = true;
    status_page = max(0,min(NUMPAGES-1,status_page));
  } else {
    status_page = start_page;
    multi_page = false;
  }

  first = true;

  char filename[MAX_PATH];
  LocalPathS(filename, TEXT("dlgStatus.xml"));
  wf = dlgLoadFromXML(CallBackTable, 
                      filename, 
		      hWndMainWindow,
		      TEXT("IDR_XML_STATUS"));

  if (!wf) return;

  wf->SetKeyDownNotify(FormKeyDown);

  ((WndButton *)wf->FindByName(TEXT("cmdClose")))->SetOnClickNotify(OnCloseClicked);

  wStatus0    = ((WndFrame *)wf->FindByName(TEXT("frmStatusFlight")));
  wStatus1    = ((WndFrame *)wf->FindByName(TEXT("frmStatusSystem")));
  wStatus2    = ((WndFrame *)wf->FindByName(TEXT("frmStatusTask")));
  wStatus3    = ((WndFrame *)wf->FindByName(TEXT("frmStatusRules")));
  wStatus4    = ((WndFrame *)wf->FindByName(TEXT("frmStatusTimes")));

  ASSERT(wStatus0!=NULL);
  ASSERT(wStatus1!=NULL);
  ASSERT(wStatus2!=NULL);
  ASSERT(wStatus3!=NULL);
  ASSERT(wStatus4!=NULL);

  wf->SetTimerNotify(OnTimerNotify);

  if (!multi_page) {
    WndButton *wb;
    wb = ((WndButton *)wf->FindByName(TEXT("cmdNext")));
    if (wb != NULL) {
      wb->SetVisible(false);
    }
    wb = ((WndButton *)wf->FindByName(TEXT("cmdPrev")));
    if (wb != NULL) {
      wb->SetVisible(false);
    }
  }

  UpdateValuesSystem();
  UpdateValuesFlight();
  UpdateValuesTask();
  UpdateValuesRules();
  UpdateValuesTimes();

  NextPage(0); // just to turn proper pages on/off

  wf->ShowModal();

  delete wf;

  wf = NULL;

}

