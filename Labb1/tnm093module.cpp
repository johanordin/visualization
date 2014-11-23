/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Created between 2005 and 2012 by The Voreen Team                   *
 * as listed in CREDITS.TXT <http://www.voreen.org>                   *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "modules/tnm093/tnm093module.h"

#include "modules/tnm093/include/tnm_datareduction.h"
#include "modules/tnm093/include/tnm_parallelcoordinates.h"
#include "modules/tnm093/include/tnm_raycaster.h"
#include "modules/tnm093/include/tnm_scatterplot.h"
#include "modules/tnm093/include/tnm_volumeinformation.h"

namespace voreen {

TNM093Module::TNM093Module()
    : VoreenModule()
{
    setName("TNM093");
    setXMLFileName("tnm093/tnm093module.xml");
    addShaderPath(getModulesPath("tnm093/glsl"));

    addProcessor(new TNMDataReduction);
    addProcessor(new TNMParallelCoordinates);
    addProcessor(new TNMRaycaster);
    addProcessor(new TNMScatterPlot);
    addProcessor(new TNMVolumeInformation);
}

} // namespace
