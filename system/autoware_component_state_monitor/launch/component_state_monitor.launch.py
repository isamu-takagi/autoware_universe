# Copyright 2022 TIER IV, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


from pathlib import Path

import launch
from launch.actions import DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch.actions import OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import ComposableNodeContainer
from launch_ros.substitutions import FindPackageShare
from launch_ros.utilities import make_namespace_absolute
from launch_ros.utilities import prefix_namespace
import yaml


def create_diag_name(row):
    return row["module"] + "_topic_status"


def create_node_name(row):
    return "topic_state_monitor_" + row["args"]["node_name_suffix"]


def create_topic_monitor_name(row):
    return create_node_name(row) + ": " + create_diag_name(row)


def create_topic_monitor_node(row, target_container):
    tf_mode = "" if "topic_type" in row["args"] else "_tf"
    package = FindPackageShare("autoware_topic_state_monitor")
    include = PathJoinSubstitution(
        [package, f"launch/load_topic_state_monitor{tf_mode}.launch.xml"]
    )
    diag_name = create_diag_name(row)
    arguments = [("diag_name", diag_name), ("target_container", target_container)] + [
        (k, str(v)) for k, v in row["args"].items()
    ]
    return IncludeLaunchDescription(include, launch_arguments=arguments)


def launch_setup(context, *args, **kwargs):
    # create container name based on current ros namespace
    target_namespace = context.launch_configurations.get("ros_namespace", None)
    target_container = make_namespace_absolute(
        prefix_namespace(target_namespace, "component_state_monitor/container")
    )

    # create topic monitors
    mode = LaunchConfiguration("mode").perform(context)
    rows = yaml.safe_load(Path(LaunchConfiguration("file").perform(context)).read_text())
    rows = [row for row in rows if mode in row["mode"]]
    topic_monitor_nodes = [create_topic_monitor_node(row, target_container) for row in rows]

    # The topic_state_monitor nodes remain composable nodes loaded into this container by name.
    container = ComposableNodeContainer(
        namespace="component_state_monitor",
        name="container",
        package="rclcpp_components",
        executable="component_container",
        composable_node_descriptions=[],
    )
    return [container, *topic_monitor_nodes]


def generate_launch_description():
    return launch.LaunchDescription(
        [
            DeclareLaunchArgument("file"),
            DeclareLaunchArgument("mode"),
            OpaqueFunction(function=launch_setup),
        ]
    )
