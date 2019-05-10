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
 * This is a template header file for a user modules derived from
 * DefaultGUIModel with a custom GUI.
 */

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <default_gui_model.h>

// in module_help
#include <eigen/Eigen/Dense>
#include <StAC_rtxi/dataFuns.h>
//#include <unsupported/Eigen/CXX11/Tensor>

// plds
#include <dynCtrlEst>
#include <plds_adam_funs.hpp>

class SsPlant : public DefaultGUIModel
{

  Q_OBJECT

public:
  SsPlant(void);
  virtual ~SsPlant(void);

  void execute(void);
  void createGUI(DefaultGUIModel::variable_t*, int);
  void customizeGUI(void);

protected:
  virtual void update(DefaultGUIModel::update_flags_t);

private:
  double some_parameter;
  double some_state;
  double period;

  int switch_idx;
  plds_adam sys;
  plds_adam sys1;
  plds_adam sys2;
  
  glds_adam sysn;
/*
        adam::Mat A; //Eigen::Matrix2d A;
        adam::Vec B; //Eigen::Vector2d B;
        adam::RowVec C; //Eigen::RowVector2d C; 
        adam::data_t D; 
 */   
        adam::Vec x;
        adam::data_t y;
        adam::data_t u;

  void switchPlant(int);

  void resetAllSys();
  void initParameters();


private slots:
  // these are custom functions that can also be connected to events
  // through the Qt API. they must be implemented in plugin_template.cpp

  void aBttn_event(void);
  void bBttn_event(void);
};
