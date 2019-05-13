/*
 * Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
 * Weill Cornell Medical College
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * do gui elements last, just get outputs to screen
 * 
 */

// SWITCHED PLANT


#include "ss_plant.h"
#include <main_window.h>

using namespace adam;//plds stuff


extern "C" Plugin::Object*
createRTXIPlugin(void)
{
  return new SsPlant();
}

static DefaultGUIModel::variable_t vars[] = {
  {
    "State-space plant", "Tooltip description",
    DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE,
  },

	{
	    "y","output", DefaultGUIModel::OUTPUT,
	},
	{ "X_out", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, },
	{
	    "yn","output", DefaultGUIModel::OUTPUT,
	},
  {
    "x1", "Tooltip description", DefaultGUIModel::OUTPUT,
  },
  {
    "x2", "Tooltip description", DefaultGUIModel::OUTPUT,
  },


	{
		"ustim","input", DefaultGUIModel::INPUT,
	},
	{
		"u dist","disturbance", DefaultGUIModel::INPUT,
	},
	{"q","state_index", DefaultGUIModel::INPUT | DefaultGUIModel::INTEGER},


};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

SsPlant::SsPlant(void)
  : DefaultGUIModel("SsPlant with Custom GUI", ::vars, ::num_vars)
{
  setWhatsThis("<p><b>SsPlant:</b><br>QWhatsThis description.</p>");
  DefaultGUIModel::createGUI(vars,
                             num_vars); // this is required to create the GUI
  customizeGUI();
  initParameters();
  update(INIT); // this is optional, you may place initialization code directly
                // into the constructor
  refresh();    // this is required to update the GUI with parameter and state
                // values
  QTimer::singleShot(0, this, SLOT(resizeMe()));
}

SsPlant::~SsPlant(void)
{
}


void SsPlant::switchPlant(int idx)
{
	x = sys.x;
	sys = ((idx==0) ? sys1 : sys2);
	sys.x = x; //make sure new system has up to date state


	multi_sys.switchSys(idx);

	//A = sys.A;
	//B = sys.B;
	//C = sys.C;
	//D = sys.D;
}

void
SsPlant::execute(void)
{
	switch_idx = input(2);
	switchPlant(switch_idx);//move into system class later

	double u_pre = input(0)+input(1);
	double u_total = u_pre;
	sys.stepPlant(u_total);
	sysn.stepPlant(u_total);
	
	//offload new sys properties
	x=sys.x;
	y=sys.y;

	setState("x1",x(0));
	setState("x2",x(1));
	
	output(0) = y;

	outputVector(1) = arma::conv_to<stdVec>::from(x);//xstd;

	output(2) = sysn.y;

	output(3) = x(0);
	output(4) = x(1);
	
  return;
}


void SsPlant::resetAllSys(void)
{
	sys.resetSys();
	sys1.resetSys();
	sys2.resetSys();
}
void
SsPlant::initParameters(void)
{
  some_parameter = 0;
  some_state = 0;
	sys = lds_adam();
	sys.initSys();

	sysn = glds_adam();
	sysn.printSys();

	sys1 = sys;
	sys2 = sys;
	sys2.B = sys2.B*1.4;


	multi_sys = slds();
	multi_sys.switchSys((int)0);
	multi_sys.switchSys(1);
	multi_sys.switchSys(-1);
	multi_sys.switchSys(2);

}

void
SsPlant::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag) {
    case INIT:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      setParameter("GUI label", some_parameter);
      //setState("A State", some_state);
      break;

    case MODIFY:
      some_parameter = getParameter("GUI label").toDouble();
      break;

    case UNPAUSE:
      break;

    case PAUSE:
      break;

    case PERIOD:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      break;

    default:
      break;
  }
}

void
SsPlant::customizeGUI(void)
{
  QGridLayout* customlayout = DefaultGUIModel::getLayout();

  QGroupBox* button_group = new QGroupBox;

  QPushButton* abutton = new QPushButton("Load Matrices");
  QPushButton* bbutton = new QPushButton("Reset Sys");
  QHBoxLayout* button_layout = new QHBoxLayout;
  button_group->setLayout(button_layout);
  button_layout->addWidget(abutton);
  button_layout->addWidget(bbutton);
  QObject::connect(abutton, SIGNAL(clicked()), this, SLOT(aBttn_event()));
  QObject::connect(bbutton, SIGNAL(clicked()), this, SLOT(bBttn_event()));

  customlayout->addWidget(button_group, 0, 0);

  setLayout(customlayout);
}

// functions designated as Qt slots are implemented as regular C++ functions
void
SsPlant::aBttn_event(void)
{
	//sys.initSys();
	initParameters();
}

void
SsPlant::bBttn_event(void)
{
	//sys.resetSys();
	resetAllSys();
}

