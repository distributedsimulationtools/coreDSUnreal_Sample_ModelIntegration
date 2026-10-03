# Copyright (C) Consultants 2J's, Inc - operating under the name ds.tools
# coreDS Unreal sample - Model Integration
#
# Creates the sample's assets from the C++ model:
#   /Game/ModelIntegration/Blueprints/BP_ModelIntegrationTank  (Blueprint deriving from AModelIntegrationTank)
#   /Game/ModelIntegration/Maps/ModelIntegrationMap            (test level with a published C++ tank and Blueprint tank)
#
# Run it with the oldest supported engine so the assets load in every later version:
#   UnrealEditor-Cmd.exe <path>\coreDSModelIntegration.uproject -run=pythonscript -script="<path>\Scripts\CreateSampleAssets.py" -unattended -nosplash
#
# Existing assets are kept; delete them first to regenerate.

import unreal

BLUEPRINT_FOLDER = "/Game/ModelIntegration/Blueprints"
BLUEPRINT_NAME = "BP_ModelIntegrationTank"
MAP_PATH = "/Game/ModelIntegration/Maps/ModelIntegrationMap"
MODEL_CLASS = "/Script/coreDSModelIntegration.ModelIntegrationTank"

# The Blueprint represents another tank than the C++ class (M1A2, 1.1.225.1.1.3), so a
# receiver spawns each remote tank with the class that sent it.
BLUEPRINT_ENTITY_TYPE = ["1.1.225.1.1.1.*"]  # M1 Abrams


def load_model_class():
    model_class = unreal.load_class(None, MODEL_CLASS)
    if model_class is None:
        raise RuntimeError("Cannot load " + MODEL_CLASS + ": build the coreDSModelIntegration module first")
    return model_class


def create_blueprint(model_class):
    blueprint_path = BLUEPRINT_FOLDER + "/" + BLUEPRINT_NAME
    if unreal.EditorAssetLibrary.does_asset_exist(blueprint_path):
        unreal.log("Keeping existing " + blueprint_path)
        return unreal.EditorAssetLibrary.load_asset(blueprint_path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", model_class)
    blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        BLUEPRINT_NAME, BLUEPRINT_FOLDER, unreal.Blueprint, factory)
    if blueprint is None:
        raise RuntimeError("Cannot create " + blueprint_path)

    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.get_default_object(blueprint.generated_class()).set_editor_property("coreDS_EntityType", BLUEPRINT_ENTITY_TYPE)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False)
    unreal.log("Created " + blueprint_path)
    return blueprint


def spawn(actor_class, location, rotation=unreal.Rotator(0, 0, 0)):
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    return actors.spawn_actor_from_class(actor_class, location, rotation)


def set_movable(actor):
    actor.root_component.set_mobility(unreal.ComponentMobility.MOVABLE)


def spawn_published_tank(tank_class, label, location):
    # coreDS_Replicate is false on the classes: only these instances are published.
    tank = spawn(tank_class, location)
    tank.set_actor_label(label)
    tank.set_editor_property("coreDS_Replicate", True)


def create_map(model_class, blueprint):
    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        unreal.log("Keeping existing " + MAP_PATH)
        return

    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not levels.new_level(MAP_PATH):
        raise RuntimeError("Cannot create " + MAP_PATH)

    # 200 m x 200 m ground, so remote tanks have something to snap to.
    floor = spawn(unreal.StaticMeshActor, unreal.Vector(0, 0, 0))
    floor.set_actor_label("Ground")
    floor.static_mesh_component.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Plane.Plane"))
    floor.set_actor_scale3d(unreal.Vector(200, 200, 1))

    # Movable lights only: no lighting build needed.
    sun = spawn(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(0, -45, 30))
    sun.set_actor_label("Sun")
    set_movable(sun)
    sun.light_component.set_editor_property("atmosphere_sun_light", True)

    spawn(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0)).set_actor_label("SkyAtmosphere")

    sky_light = spawn(unreal.SkyLight, unreal.Vector(0, 0, 500))
    sky_light.set_actor_label("SkyLight")
    set_movable(sky_light)
    sky_light.light_component.set_editor_property("real_time_capture", True)

    player_start = spawn(unreal.PlayerStart, unreal.Vector(-5000, 0, 1500), unreal.Rotator(0, -15, 0))
    player_start.set_actor_label("PlayerStart")

    # One tank of each kind, each driving its own circle.
    spawn_published_tank(model_class, "LocalTank_Cpp", unreal.Vector(0, -2500, 0))
    spawn_published_tank(blueprint.generated_class(), "LocalTank_Blueprint", unreal.Vector(0, 2500, 0))

    levels.save_current_level()
    unreal.log("Created " + MAP_PATH)


model = load_model_class()
create_map(model, create_blueprint(model))
