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


void SsPlant::stepPlant(double uin)
{
	u = uin;
	x = A*x + B*u;
	y = C*x;
}

void
SsPlant::execute(void)
{
	stepPlant(input(0)+input(1));
	setState("x1",x(0));
	setState("x2",x(1));
	
	output(0) = y;
	output(1) = x(0);
	output(2) = x(1);
  return;
}


void
SsPlant::loadSys(void)
{	
	std::ifstream myfile;
	myfile.open("../ss_ctrl/params/plant_params.txt");
	// numA;
	//halp::simpleFun();
	pullParamLine(myfile); //gets nx

	std::vector<double> numA = pullParamLine(myfile); 	
	Eigen::Map<Eigen::Matrix2d> tA(numA.data(),A.rows(),A.cols());
	A = tA;
	
	std::vector<double> numB = pullParamLine(myfile); 	
	Eigen::Map<Eigen::Vector2d> tB(numB.data(),B.rows(),1);
	B = tB;

	std::vector<double> numC = pullParamLine(myfile); 	
	Eigen::Map<Eigen::RowVector2d> tC(numC.data(),1,C.cols());
	C = tC;

	//For some silly reason, can't load D this way
	std::vector<double> numD = pullParamLine(myfile); 	
	//std::cout <<"ww"<< *numD.begin()<<"ww\n";
	D = numD[0];
	//D = (float) numD.at(0);
	
	myfile.close();
/*
	//look on stackoverflow @ initialize eigenvector with stdvector
	float data[] = {1,2,3,4};
	Eigen::Map<Eigen::Vector3f> v1(data);
	std::vector<float> data2= {1,2,3,4};
	Eigen::Vector3f v2(data2.data());
	std::cout<<v2<<"?\n";
	//Eigen::Matrix2f zz;
*/
}

void SsPlant::printSys(void)
{
  std::cout <<"Here is the matrix A:\n" << A << "\n";
  std::cout <<"Here is the matrix B:\n" << B << "\n";
  std::cout <<"Here is the matrix C:\n" << C << "\n";
  std::cout <<"Here is the matrix D:\n" << D << "\n";
}

void SsPlant::resetSys(void)
{
	x << 0,0;
	y = 0;
	u = 0;
}


void
SsPlant::initParameters(void)
{
  some_parameter = 0;
  some_state = 0;

/*
	A << 0.9990, 0.0095,
		-0.1903, 0.9039;
	B << 0,
		 0.0095;
	C << 1,0;
	D=0;
*/

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

