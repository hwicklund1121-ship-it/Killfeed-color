class CfgPatches
{
    class blake
    {
        units[] = {"blake_SurgicalGloves","blake_jean","blake_helment","blake_sneakers","blake_Tshirt"};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data","DZ_Characters","DZ_Characters_Tops","DZ_Characters_Pants","DZ_Characters_Gloves","DZ_Characters_Shoes","DZ_Characters_Headgear","DZ_Characters_Vests","UAV_Helmet_BNTI","sk_taladro"};
    };
};


class CfgVehicles
{
    class SurgicalGloves_ColorBase;
    class Jeans_Black;
    class sk_taladro_tshirtn_ColorBase;
    class Sneakers_ColorBase;
    class UAV_Helmet_BNTI_ColorBase;

    class blake_SurgicalGloves: SurgicalGloves_ColorBase
    {
        scope = 2;
        displayName = "blakes Surgical Gloves";
        descriptionShort = "Designed by blake";
        hiddenSelectionsTextures[] =
        {
            "Blakes_Kit\data\gloves\blakes_Sgloves.paa",
            "Blakes_Kit\data\gloves\blakes_Sgloves.paa",
            "Blakes_Kit\data\gloves\blakes_Sgloves.paa"
        };
        hiddenSelectionsMaterials[] =
        {
            "Blakes_Kit\data\gloves\blakes_Sgloves.rvmat",
            "Blakes_Kit\data\gloves\blakes_Sgloves.rvmat",
            "Blakes_Kit\data\gloves\blakes_Sgloves.rvmat"
        };
    };

    class blake_jean: Jeans_Black
    {
        scope = 2;
        displayName = "blakes Jeans";
        descriptionShort = "Designed by blake";

        hiddenSelectionsTextures[] =
        {
            "Blakes_Kit\data\Jeans\Blakes_Jeans.paa",
            "Blakes_Kit\data\Jeans\Blakes_Jeans.paa",
            "Blakes_Kit\data\Jeans\Blakes_Jeans.paa"
        };
        hiddenSelectionsMaterials[] =
        {
            "Blakes_Kit\data\Jeans\Blakes_Jeans.rvmat",
            "Blakes_Kit\data\Jeans\Blakes_Jeans.rvmat",
            "Blakes_Kit\data\Jeans\Blakes_Jeans.rvmat"
        };
    };

    class blake_helment : UAV_Helmet_BNTI_ColorBase
    {
        scope = 2;
        displayName = "Blakes Helmet";
        descriptionShort = "Designed by Blake";
        hiddenSelections[] = {"camo"};
        hiddenSelectionsTextures[] = {"Blakes_Kit\data\helment\blake_helmet_co.paa"};
        hiddenSelectionsMaterials[] = {"Blakes_Kit\data\helment\blake_helmet.rvmat"};
    };

    class blake_sneakers: Sneakers_ColorBase
    {
        scope = 2;
        displayName = "Blakes Sneakers";
        descriptionShort = "Designed by Blake";

        hiddenSelectionsTextures[] =
        {
            "Blakes_Kit\data\airforce1\Blakes_Sneakers.paa",
            "Blakes_Kit\data\airforce1\Blakes_Sneakers.paa",
            "Blakes_Kit\data\airforce1\Blakes_Sneakers.paa"
        };
        hiddenSelectionsMaterials[] =
        {
            "Blakes_Kit\data\airforce1\Blakes_Sneakers.rvmat",
            "Blakes_Kit\data\airforce1\Blakes_Sneakers.rvmat",
            "Blakes_Kit\data\airforce1\Blakes_Sneakers.rvmat"
        };
    };

    class blake_Tshirt: sk_taladro_tshirtn_ColorBase
    {
        scope = 2;
        displayName = "blakes T-Shirt";
        descriptionShort = "Designed by blake";
        hiddenSelectionsTextures[] =
        {
            "Blakes_Kit\data\Tshirt\Blake_Tshirt.paa"
        };
        hiddenSelectionsMaterials[] =
        {
            "Blakes_Kit\data\Tshirt\Blake_Tshirt.rvmat"
        };
    };
};