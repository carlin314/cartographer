/*
 * Copyright 2016 The Cartographer Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CARTOGRAPHER_INTERNAL_MAPPING_3D_SCAN_MATCHING_OCCUPIED_SPACE_COST_FUNCTION_H_
#define CARTOGRAPHER_INTERNAL_MAPPING_3D_SCAN_MATCHING_OCCUPIED_SPACE_COST_FUNCTION_H_

#include "Eigen/Core"
#include "cartographer/mapping_3d/hybrid_grid.h"
#include "cartographer/mapping_3d/scan_matching/interpolated_grid.h"
#include "cartographer/sensor/point_cloud.h"
#include "cartographer/transform/rigid_transform.h"
#include "cartographer/transform/transform.h"

namespace cartographer {
namespace mapping_3d {
namespace scan_matching {

// Computes a cost for matching the 'point_cloud' to the 'hybrid_grid' with a
// 'translation' and 'rotation'. The cost increases when points fall into less
// occupied space, i.e. at voxels with lower values.
class OccupiedSpaceCostFunction {
 public:
  OccupiedSpaceCostFunction(const double scaling_factor,
                            const sensor::PointCloud& point_cloud,
                            const HybridGrid& hybrid_grid)
      : scaling_factor_(scaling_factor),
        point_cloud_(point_cloud),
        interpolated_grid_(hybrid_grid) {}

  OccupiedSpaceCostFunction(const OccupiedSpaceCostFunction&) = delete;
  OccupiedSpaceCostFunction& operator=(const OccupiedSpaceCostFunction&) =
      delete;

  template <typename T>
  bool operator()(const T* const translation, const T* const rotation,
                  T* const residual) const {
    const transform::Rigid3<T> transform(
        Eigen::Map<const Eigen::Matrix<T, 3, 1>>(translation),
        Eigen::Quaternion<T>(rotation[0], rotation[1], rotation[2],
                             rotation[3]));
    return Evaluate(transform, residual);
  }

  template <typename T>
  bool Evaluate(const transform::Rigid3<T>& transform,
                T* const residual) const {
    for (size_t i = 0; i < point_cloud_.size(); ++i) {
      const Eigen::Matrix<T, 3, 1> world =
          transform * point_cloud_[i].cast<T>();
      const T probability =
          interpolated_grid_.GetProbability(world[0], world[1], world[2]);
      residual[i] = scaling_factor_ * (1. - probability);
    }
    return true;
  }

 private:
  const double scaling_factor_;
  const sensor::PointCloud& point_cloud_;
  const InterpolatedGrid interpolated_grid_;
};

}  // namespace scan_matching
}  // namespace mapping_3d
}  // namespace cartographer

#endif  // CARTOGRAPHER_INTERNAL_MAPPING_3D_SCAN_MATCHING_OCCUPIED_SPACE_COST_FUNCTION_H_
