coreDSModelIntegration
======================

This is a Sample project to be used with coreDS Unreal and Unreal Engine 5. You can request a free trial at https://www.ds.tools/contact-us/trial-request/

coreDS Unreal must already be installed and activated to use this project. Please make sure the coreDS Unreal plugin is enabled.

This sample is compatible with Unreal Engine 5.2 and later. Its assets are saved with Unreal Engine 5.2. Set the engine of ``coreDSModelIntegration.uproject`` to the version your coreDS Unreal plugin was built for.

This sample showcases the Model Integration approach (see "Automatic Object Support / Content provider support" in the coreDS Unreal user guide). A model written in C++ declares which variables, articulated parts, appearance flags and events it supports. coreDS Unreal discovers the model, spawns it for remote entities, drives it from DIS or HLA, and publishes the local instances, without any coreDS-specific code in the model.

The model is used two ways, side by side: directly as a C++ class, and through a Blueprint deriving from it. Both are discovered by coreDS Unreal and published to DIS, and a receiver spawns each remote tank with the class that sent it.

The model does not depend on coreDS Unreal: it uses the "loose coupling" approach, with its own copies of the Model Integration data structures. The project builds and runs without the plugin; the plugin is only needed to connect to DIS or HLA.

"Loose coupling" allows the model to be developed and tested independently of the coreDS Unreal plugin, ensuring that the coreDS-specific code is only required when connecting to DIS or HLA. This approach promotes modularity and makes it easier to maintain and extend the model without being tightly coupled to the coreDS Unreal plugin.

Model creators can then develop their models independently and market them as standalone products, without requiring their customers to have the coreDS Unreal plugin installed unless they want to connect to DIS or HLA.

Contents
--------

``Source/coreDSModelIntegration/Public/ModelIntegrationTank.h``
   ``AModelIntegrationTank``, a simple tank (hull, turret, gun, headlights) built from the engine's basic shapes.

``Source/coreDSModelIntegration/Public/ModelIntegrationDataStructures_V1.h``
   The local copies of the Model Integration data structures. Their field names, types and order match ``coreDSActorGenericArticulations_datatypes_V1.h``: coreDS Unreal reads ``coreDS_Variables`` and ``coreDS_Appearance`` with the plugin's own structure layout.

``Content/ModelIntegration/Blueprints/BP_ModelIntegrationTank``
   A Blueprint deriving from ``AModelIntegrationTank``. It only changes ``coreDS_EntityType``, so it represents another tank than the C++ class.

``Content/ModelIntegration/Maps/ModelIntegrationMap``
   A test level with a ground plane and two published tanks: ``LocalTank_Cpp`` (the C++ class) and ``LocalTank_Blueprint`` (the Blueprint).

``Content/coreDS/DIS_Player1.coreDS``, ``Content/coreDS/DIS_Player2.coreDS``
   DIS configurations publishing and receiving the tanks (see `DIS configuration`_).

``Scripts/CreateSampleAssets.py``
   Re-creates the Blueprint and the map from the C++ model (see `Regenerating the assets`_).

What the model exposes
----------------------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Variable
     - Content
   * - ``coreDS_EntityType``
     - C++ class: ``1.1.225.1.1.3.*`` (M1A2 Abrams), ``1.1.225.1.*.*.*``, ``1.1.*.1.*.*.*``. Blueprint: ``1.1.225.1.1.1.*`` (M1 Abrams).
   * - ``coreDS_Variables``
     - The four variables below
   * - ``coreDS_Parts``
     - Primary turret #1 azimuth (4096 + 11) from ``TurretAzimuth``, primary gun #1 elevation (4416 + 13) from ``GunElevation``
   * - ``coreDS_Appearance``
     - ``DamageState`` (bits 3-4) from ``Damage``, ``HeadLightsOn`` (bit 12) from ``Headlights``
   * - ``coreDS_Replicate``
     - False on the classes, true on the two tanks placed in the level: only these are published.
   * - ``coreDS_SnapToGround``
     - True
   * - ``coreDS_Event_Fire``
     - ``OnCoreDSFire(FVector Location)``
   * - ``coreDS_Event_Detonation``
     - ``OnCoreDSDetonation(FVector Location, FVector LocationOnEntity)``
   * - ``coreDS_IntegrationVersion``
     - ``3.0.0``

Each variable uses a different access mechanism from the specification:

.. list-table::
   :header-rows: 1
   :widths: 18 32 15 35

   * - Name
     - Access
     - Model range
     - What it tests
   * - ``TurretAzimuth``
     - UPROPERTY ``TurretAzimuth`` + callback ``OnTurretAzimuthUpdated``
     - -π to π rad
     - Direct property access, callback after a write
   * - ``GunElevation``
     - Setter ``SetGunElevation`` + getter ``GetGunElevation``, no UPROPERTY
     - -0.17 to 0.35 rad
     - Getter/setter-only access
   * - ``Damage``
     - UPROPERTY ``DamagePercent`` (int32) + callback ``OnDamageUpdated``
     - 0 to 100
     - Value approximation from 0-100 to the 0-3 DamageState
   * - ``Headlights``
     - UPROPERTY ``bHeadlightsOn`` (bool) + callback ``OnHeadlightsUpdated``
     - 0 to 1
     - Boolean appearance flag

A tank that coreDS did not spawn (no ``coreDSCreated`` tag) simulates its own activity so every outgoing value changes. It drives in a 15 m circle from a random starting point and sweeps the turret and gun. Its damage rises from 0 to 100 % over 40 seconds, which changes the hull colour from green to red, then black when destroyed. Its headlights toggle every 5 seconds. A tank that coreDS spawned for a remote entity is driven only by coreDS. Turn ``bSimulateLocalActivity`` off on a placed tank to keep it still.

A label above each tank shows whether it is the C++ class or the Blueprint, and whether it is local or a remote entity spawned by coreDS.

Getting started
---------------

#. Install coreDS Unreal in the engine or in the project's ``Plugins`` folder, then open ``coreDSModelIntegration.uproject`` and rebuild when asked.
#. Run Tools > Distributed Simulation > coreDS Unreal > "Search for compatible assets ...". Both ``BP Model Integration Tank`` (the Blueprint) and ``Model Integration Tank`` (the C++ class) must be reported and listed in Edit > Project Settings > coreDS Unreal > Automatic Mode, the Blueprint first. The project already contains these entries (``Config/DefaultcoreDS.ini``), so this step checks that the discovery finds the model.
#. Select ``DIS_Player1`` in the coreDS window and set the network adapter and the destination address for your network (see `DIS configuration`_).
#. Play.

DIS configuration
-----------------

Every tank published by coreDS Unreal is the local object ``AutomaticMode``, whatever its class. One mapping, ``AutomaticMode`` to ``Entity State``, therefore publishes both the C++ tank and the Blueprint tank:

- EntityType and AlternativeEntityType come from the first ``coreDS_EntityType`` entry of the tank's class (``EntityType_*`` variables). The two tanks are sent with different types: 1.1.225.1.1.3.0 and 1.1.225.1.1.1.0.
- EntityID is generated for each tank, from the configured site and application numbers.
- Marking is the actor name (``ActorName``).
- Appearance and VariableParameters come from ``coreDS_Appearance`` and ``coreDS_Parts`` (``Appearance``, ``ArticulatedParts``, ``NumberOfParts``).
- Location and Orientation are converted from the flat Unreal world to geocentric coordinates by ``convertUnrealPositionToDIS.lua`` and ``convertOrientationFromUnrealToDIS.lua``, around the reference of ``ReferenceLatLongAlt.lua``.
- Dead reckoning is static (algorithm 1), with no velocity: the ``DR_*`` variables are in Unreal units and are not converted.

Incoming Entity State PDUs are mapped to ``AutomaticMode``. The EntityType becomes the ``ObjectClassSelector`` (``UseEntityTypeAsUniqueIdentifier.lua``), which selects the class to spawn: the first Automatic Mode mapping matching it is used, so 1.1.225.1.1.1.x spawns the Blueprint and 1.1.225.1.1.3.x spawns the C++ class. Location and orientation are converted back by ``convertPositionFromDISToUnreal.lua``, and Appearance and VariableParameters drive the tank's appearance and parts.

The two configurations publish and receive the same objects. They use different application numbers (3001 and 3002), so two instances on the same network do not send the same EntityIDs. Before using them, set in the coreDS window:

- The network adapter (``DISNIA``) and the destination address (``DISDestinationAddress``): ``DIS_Player1`` is set for a 192.168.50.x network and ``DIS_Player2`` for a 192.168.2.x network.
- The FOM file, which is an absolute path to ``Plugins\coreDSModule\ThirdParty\coreDS\bin\VC143\x64\Release\DIS_V5_IEEE_1278.1-1995_expert.xml`` in this sample's folder. Change it if the sample or the plugin is elsewhere.

Testing both directions
-----------------------

Run two instances of the sample on the same network, one with ``DIS_Player1`` and the other with ``DIS_Player2``. For example, use Play In Editor and a Standalone Game, or two packaged copies.

Each instance publishes its two tanks and receives the other's. Check that:

- Each received tank is labelled "remote (coreDS)", with the same class as the tank that sent it: "C++ class" for ``LocalTank_Cpp``, "Blueprint" for ``LocalTank_Blueprint``.
- It follows the other instance's tank around its circle (location and orientation).
- Its turret and gun move like the other instance's (articulated parts, ``TurretAzimuth`` and ``GunElevation``).
- Its hull colour changes in steps: DIS sends the damage as 0-3, which coreDS converts back to 0, 33, 67 and 100 % (appearance with value approximation).
- Its headlights follow the other instance's (boolean appearance).
- The log shows ``LogModelIntegration`` messages for the remote tanks only, after ``log LogModelIntegration Verbose``.

coreDS Unreal publishes Automatic Mode objects once per second, and the dead reckoning is static, so the received tanks move in steps.

To test the events, send a Fire or Detonation PDU from another simulation. The tank logs the event and draws a sphere at its location.

You can also test one direction at a time with any DIS tool sending or receiving entity types ``1.1.225.1.1.3.0`` and ``1.1.225.1.1.1.0``.

The HLA configurations (``HLA_Player1.coreDS``, ``HLA_Player2.coreDS``) come from the Automatic Mode sample and do not publish the tanks yet.
