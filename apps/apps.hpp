#pragma once
#include "core/cli.hpp"
namespace g101 {
bool run_raytrace(const Args& args, bool pathtrace);
bool run_raster  (const Args& args);
bool run_bezier  (const Args& args);
bool run_rope    (const Args& args);
}