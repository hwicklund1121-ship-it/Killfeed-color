CSR50 Reskin Kit

Open CSR50_Reskin.blend in Blender, or import CSR50_Reskin.fbx into your painting application. OBJ/MTL is included as a fallback.

The original archive did not contain an FBX. This FBX is rebuilt from the highest-detail visual LOD of the supplied editable CSR50 MLOD. The unusable constant secondary UV channel is removed. The primary UV map is shifted back by one V tile into 0..1, preserving the existing repeating texture alignment. Geometry, original normals, scale and topology are retained; six attachment proxy triangles are excluded from the artist FBX. Other LODs are kept in the editable P3D source rather than stacked into the FBX.

Paint textures/part1_co.png. CSR50_UV_Layout_2048.png is the transparent UV wireframe overlay. UV island positions, seams and existing mirrored/shared areas are retained, so existing base colour and normal textures remain aligned. The Blender file contains a single material with a clearly labelled PAINT THIS base-colour image and packed textures. The FBX embeds its base-colour and preview-normal textures; external copies are also provided.

textures/part1_nohq.png and part1_smdi.png are decoded originals, including the PAA channel-swizzle metadata. part1_normal_blender.png has its green channel inverted for the Blender normal preview. Keep the original DayZ NOHQ and SMDI maps for a colour-only reskin. SMDI is a reference node in Blender, because the DayZ Super shader is not a direct Principled roughness map; Blender shading is an approximation.

DayZ_Model_Source/CSR50.p3d is the editable MLOD with repaired primary UV coordinates and the unused secondary visual UV channel removed. All 9 LODs, vertices, faces, normals, bone selections, memory points, proxy geometry and non-UV tag bytes are preserved. model.cfg is included. FBX is an artist mesh, not a complete animated DayZ weapon source; use the supplied MLOD for the complete weapon structure.

GRZ_CSR50_Reskin is an optional unpacked reskin addon example. It inherits UAV_CSR50 from the original CSR50_Base patch, uses the existing weapon model, and sets both zbytek/camo selections. Replace its data/part1_co.paa after painting and converting your PNG with DayZ Tools. The example RVMAT points at its own included NOHQ and SMDI files. The original CSR50 addon remains required. The one-tile UV translation preserves repeating texture appearance, so a colour-only reskin can use the existing compiled model. No gameplay, ammunition, sounds, stats or server updates are bundled here.

Verified: independent FBX reimport with one mesh/material/UV channel; identical face count, dimensions and UV coordinates; MLOD geometry/normals/face topology retained; raw non-UV tag data retained; textures load and material preview rendered. The reskin config and MLOD still require DayZ Tools compilation and an in-game test before deployment.
