/************************************************************************
 *                                                                      *
 * Copyright (C) 2012 OVSM/IPGP                                         *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.     *
 * It has been co-financed by the European Union and le Ministère de    *
 * l'Ecologie, du Développement Durable, des Transports et du Logement. *
 *                                                                      *
 ************************************************************************/


#ifndef __IPGP_OPENGL_DATAMODEL_MATH_H__
#define __IPGP_OPENGL_DATAMODEL_MATH_H__

#include <ipgp/gui/api.h>
#include <ipgp/gui/defs.h>


namespace IPGP {
namespace Gui {
namespace OpenGL {

DEFINE_IPGP_SMARTPOINTER(Vertex);

namespace Math {

float SC_IPGP_GUI_API rad2deg(const float& angle);
float SC_IPGP_GUI_API deg2rad(const float& angle);
float SC_IPGP_GUI_API pow2(const float& val);

float SC_IPGP_GUI_API SIN(const float& ang);
float SC_IPGP_GUI_API COS(const float& ang);
float SC_IPGP_GUI_API TAN(const float& ang);

float SC_IPGP_GUI_API ASIN(const float& val);
float SC_IPGP_GUI_API ACOS(const float& val);
float SC_IPGP_GUI_API ATAN(const float& val);
float SC_IPGP_GUI_API ATAN2(const float& val1, const float& val2);

Vertex SC_IPGP_GUI_API getCartesian(const float& rho, const float& phi, const float& theta);

Vertex SC_IPGP_GUI_API triangleNormal(const Vertex& v1, const Vertex& v2, const Vertex& v3);
Vertex SC_IPGP_GUI_API quadNormal(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4);

float SC_IPGP_GUI_API dot(const Vertex& v1, const Vertex& v2);
Vertex SC_IPGP_GUI_API cross(const Vertex& v1, const Vertex& v2);

Vertex SC_IPGP_GUI_API mix(const Vertex& v1, const Vertex& v2, const float& porc);

} // namespace OpenGL
} // namespace Math
} // namespace Gui
} // namespace IPGP


#endif
