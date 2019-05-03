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
	if (idx==0)
	{
		A=A_;
		B=B_;
		C=C_;
		D=D_;
	}
	else
	{
		A=A2;
		B=B2;
		C=C2;
		D=D2;
	}
}
void SsPlant::stepPlant(double uin)
{

	//A = As(
	u = uin;
	x = A*x + B*u; //index here
	y = C*x; //index here
}

void
SsPlant::execute(void)
{

	switch_idx = input(2);
	switchPlant(switch_idx);

	double u_pre = input(0)+input(1);
	double u_total = u_pre; //+u_fromvec
	stepPlant(u_total);
	setState("x1",x(0));
	setState("x2",x(1));
	
	output(0) = y;

	std::vector<double>xstd(x.data(),x.data()+x.size());

	outputVector(1) = xstd;
	output(2) = x(0);
	output(3) = x(1);
	
  return;
}


void
SsPlant::loadSys(void)
{	

	std::string homepath = getenv("HOME");
	std::ifstream myfile;
	myfile.open(homepath+"/RTXI/modules/ss_modules/ss_ctrl/params/plant_params.txt");

	//std::cout<<"load works here"<<"\n";
	// numA;
	//halp::simpleFun();
	pullParamLine(myfile); //gets nx


	A = stdVec2EigenM(pullParamLine(myfile), A.rows(), A.cols());
	B = stdVec2EigenV(pullParamLine(myfile), B.rows());
	C = stdVec2EigenRV(pullParamLine(myfile), C.cols());

	std::vector<double> numD = pullParamLine(myfile); 	
	D = numD[0];
	

	//hardcoding second system
	A2=A_;
	B2=B_*1.4;
	C2=C_;
	D2=D_;


	myfile.close();

}

void SsPlant::printSys(void)
{
  std::cout <<"Here is the matrix A:\n" << A_ << "\n";
  std::cout <<"Here is the matrix B:\n" << B_ << "\n";
  std::cout <<"Here is the matrix C:\n" << C_ << "\n";
  std::cout <<"Here is the matrix D:\n" << D_ << "\n";
}

void SsPlant::resetSys(void)
{

	x << 0,0;//hardcode
	y = 0;
	u = 0;
	switch_idx = 0;
}


void
SsPlant::initParameters(void)
{
  some_parameter = 0;
  some_state = 0;

	loadSys();
	printSys();
	resetSys();
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
	loadSys();
	printSys();
}

void
SsPlant::bBttn_event(void)
{
	resetSys();
}

