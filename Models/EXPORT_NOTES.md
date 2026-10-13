# Scene export for Lab12

Source: `../Blender/FutureMe.blend`. The source blend is unchanged.

## Files

- `Models/*.obj`: one file per scene collection/material combination.
- `Models/*.mtl`: exported materials with relative texture references.
- `Textures/*.png`: external copies of available material images.
- `Models/scene_manifest.json`: source objects, materials, images, geometry counts, and settings.

All requested options were enabled: Selected Objects, UV Coordinates, Normals, Triangulated Mesh, Apply Modifiers, Apply Transform, Materials. Curves as NURBS was also enabled; this scene exports mesh geometry.

## Using the assets

OBJ coordinates include the scene placement and modifier results. Axis conversion is -Z forward / Y up, scale 1: Blender (x,y,z) becomes OBJ (x,z,-y). Start with an identity model matrix, then position the camera for the scene's existing units. Do not apply the Blender object transforms again.

The Lab12 loader does not read MTL. Load each OBJ and bind its `primary_base_color_image` from the manifest before drawing it. For parts without an image, use their material color and/or a white texture. For Fred, the texture scale/offset has been baked into exported UVs so that the Lab12 loader does not need to interpret MTL mapping options.

Extra AO/normal/roughness/emission images are retained where available, but Lab12 does not use them automatically. Complex Blender materials cannot be reproduced completely by MTL alone.

Objects without UVs received export-only planar UVs. This does not change the source blend. Missing source images (candle flame and candle normal texture) are listed in the manifest and have not been replaced.

Keep asset download sources and licenses for the Google Classroom references.

