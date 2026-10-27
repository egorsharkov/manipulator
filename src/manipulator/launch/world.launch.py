import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    pkg = get_package_share_directory('manipulator')
    gz_ros = get_package_share_directory('gazebo_ros')

    world = os.path.join(pkg, 'worlds', 'manipulator_world.world')
    xacro_file = os.path.join(pkg, 'urdf', 'manipulator.urdf.xacro')
    robot_description = ParameterValue(
        Command(['xacro ', xacro_file, ' use_control:=true']), value_type=str)

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(gz_ros, 'launch', 'gazebo.launch.py')),
        launch_arguments={'world': world,
                          'gui': LaunchConfiguration('gui')}.items())

    rsp = Node(
        package='robot_state_publisher', executable='robot_state_publisher',
        parameters=[{'robot_description': robot_description,
                     'use_sim_time': True}])

    spawn = Node(
        package='gazebo_ros', executable='spawn_entity.py', output='screen',
        arguments=['-topic', 'robot_description', '-entity', 'manipulator',
                   '-x', '0', '-y', '0', '-z', '0'])

    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster"],
    )

    arm_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["arm_controller"], 
    )

    gripper_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["gripper_controller"],
    )

    return LaunchDescription([
        DeclareLaunchArgument('gui', default_value='true'),
        gazebo, rsp, spawn, joint_state_broadcaster_spawner, arm_controller_spawner, gripper_controller_spawner
    ])