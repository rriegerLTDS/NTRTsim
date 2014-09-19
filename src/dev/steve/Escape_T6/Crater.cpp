/*
 * Copyright © 2014, United States Government, as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All rights reserved.
 * 
 * The NASA Tensegrity Robotics Toolkit (NTRT) v1 platform is licensed
 * under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 */

/**
 * @file Crater.cpp
 * @brief Contains the implementation of class Crater.
 * $Id$
 */

// This module
#include "Crater.h"
// This library
#include "core/tgRod.h"
#include "core/tgBox.h"
#include "tgcreator/tgBuildSpec.h"
#include "tgcreator/tgBoxInfo.h"
#include "tgcreator/tgStructure.h"
#include "tgcreator/tgStructureInfo.h"
// The Bullet Physics library
#include "LinearMath/btVector3.h"
// The C++ Standard Library
#include <stdexcept>

namespace
{
    // similarly, frictional parameters are for the tgRod objects.
    const struct Config
    {
        double width;
        double height;
        double density;
        double friction;
        double rollFriction;
        double restitution;
    } c =
    {
        10.0, // width (dm?)
        10.0,   // height (dm?)
        0.0,      // density (kg / length^3)
        1.0,      // friction (unitless)
        0.01,     // rollFriction (unitless)
        0.2,      // restitution (?)
    };
} // namespace

Crater::Crater() : tgModel() 
{
    origin = btVector3(0,0,0);
}

Crater::Crater(btVector3 center) : tgModel() 
{
    origin = btVector3(center.getX(), center.getY(), center.getZ());
}

Crater::~Crater()
{
}
              
// Nodes: center points of opposing faces of rectangles
void Crater::addNodes(tgStructure& s)
{                                      
    const int nBoxes = 4; 
    const double shift = 20;
    const double vshift = 2;
    const double node_h = c.height/2 + vshift;
    const double node_w = c.width/2;

    // Box 1
    nodePositions.push_back(btVector3(-shift-node_w, -node_h, -shift-node_w));
    nodePositions.push_back(btVector3( shift+node_w,  node_h,  shift+node_w));
    // Box 2
    nodePositions.push_back(btVector3(-shift-node_w, -node_h, -shift-node_w));
    nodePositions.push_back(btVector3( shift+node_w,  node_h,  shift+node_w));
    // Box 3
    nodePositions.push_back(btVector3(-shift-node_w, -node_h, -shift-node_w));
    nodePositions.push_back(btVector3( shift+node_w,  node_h,  shift+node_w));
    // Box 4
    nodePositions.push_back(btVector3(-shift-node_w, -node_h, -shift-node_w));
    nodePositions.push_back(btVector3( shift+node_w,  node_h,  shift+node_w));

    // Accumulating rotation on boxes
    btVector3 rotationPoint = btVector3(0, 0, 0); // origin
    btVector3 rotationAxis = btVector3(0, 1, 0);  // y-axis
    double rotationAngle = M_PI/2;

    // 2 * nBoxes == nodePositions.size()
    for(int i=0;i<nodePositions.size();i+=2) {
        s.addNode(nodePositions[i][0],nodePositions[i][1],nodePositions[i][2]);
        s.addNode(nodePositions[i+1][0],nodePositions[i+1][1],nodePositions[i+1][2]);
        s.addRotation(rotationPoint, rotationAxis, rotationAngle);
        s.addPair(i, i+1, "box");
    }
    s.move(btVector3(0, -5, 0));
}
                      
void Crater::setup(tgWorld& world) {

    const tgBox::Config boxConfig(c.width, c.height, c.density, c.friction, c.rollFriction, c.restitution);

    // Start creating the structure
    tgStructure s;
    addNodes(s);

    // Create the build spec that uses tags to turn the structure into a real model
    tgBuildSpec spec;
    spec.addBuilder("box", new tgBoxInfo(boxConfig));

    // Create your structureInfo
    tgStructureInfo structureInfo(s, spec);

    // Use the structureInfo to build ourselves
    structureInfo.buildInto(*this, world);

    // call the onSetup methods of all observed things e.g. controllers
    notifySetup();

    // Actually setup the children
    tgModel::setup(world);
}

void Crater::step(double dt) {
    // Precondition
    if (dt <= 0.0) {
        throw std::invalid_argument("dt is not positive");
    }
    else { //Notify observers (controllers) of the step so that they can take action
        notifyStep(dt);
        tgModel::step(dt); // Step any children
    }
}

void Crater::onVisit(tgModelVisitor& r) {
    tgModel::onVisit(r);
}

void Crater::teardown() {
    notifyTeardown();
    tgModel::teardown();
}

