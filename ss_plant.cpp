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

	{ "y","output", DefaultGUIModel::OUTPUT,},
	{ "y_gauss","output", DefaultGUIModel::OUTPUT,},
	{ "y_switch","output", DefaultGUIModel::OUTPUT,},

	{"exp(y)_poisson",".", DefaultGUIModel::OUTPUT,},
	{"spikes",".", DefaultGUIModel::OUTPUT,},
	{"exp(y)_poisson_switch","",DefaultGUIModel::OUTPUT,},
	{"switch_spikes",".",DefaultGUIModel::OUTPUT,},

	{ "X_out", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, },
	{ "X_gauss", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, },
	{"X_poiss" , ".", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, },
	{ "X_switch", "testVec", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE, },
	{ "X_switch_p",".", DefaultGUIModel::OUTPUT | DefaultGUIModel::VECTORDOUBLE},
	//note that since we have 2 parallel switched systems, even though their internal dynamics are the same, internal noise processes are parallel process	
	
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


void
SsPlant::execute(void)
{
	switch_idx = input(2);
	multi_sys.switchSys(switch_idx);//move into system class later
	multi_psys.switchSys(switch_idx);

	double u_pre = input(0)+input(1);
	double u_total = u_pre;

	sys.stepPlant(u_total);
	gsys.stepPlant(u_total);
	multi_sys.stepPlant(u_total);
	
	psys.stepPlant(u_total);
	multi_psys.stepPlant(u_total);
	
	//offload new sys properties
	x=sys.x;
	y=sys.y;
	
	output(0) = y;
	output(1) = gsys.y;
	output(2) = multi_sys.y;

	output(3) = psys.y_nl;
	output(4) = psys.z;

	output(5) = multi_psys.y_nl;
	output(6) = multi_psys.z;

	

	outputVector(7) = arma::conv_to<stdVec>::from(x);
	outputVector(8) = arma::conv_to<stdVec>::from(gsys.x);
	outputVector(9) = arma::conv_to<stdVec>::from(psys.x);

	outputVector(10) = arma::conv_to<stdVec>::from(multi_sys.x);
	outputVector(11) = arma::conv_to<stdVec>::from(multi_psys.x);
  return;
}


void SsPlant::resetAllSys(void)
{
	sys.resetSys();
	gsys.resetSys();
	multi_sys.resetSys();
}
void
SsPlant::initParameters(void)
{
    sys = lds_adam();
/*
    gsys = glds_adam();
    multi_sys = slds();

    double period_in_s = (RT::System::getInstance()->getPeriod())*1e-9;
    psys = plds_adam();psys.setDt(period_in_s);
    multi_psys = splds();
*/
    sys.augment4PI();
	std::cout<<"\ndid this update??????\n";
}

void
SsPlant::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag) {
    case INIT:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      break;

    case MODIFY:
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
	initParameters();
}

void
SsPlant::bBttn_event(void)
{
	resetAllSys();
}

