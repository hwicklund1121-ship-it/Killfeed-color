GRZ No Leg Breaks and Foot Cuts 0.1.0

Stop DayZ and the server. Copy GRZ_NoLegBreaks.pbo into @GRZ_Local/Addons on
BOTH installations. Restart both, loading the existing @GRZ_Local mod. No new
launch argument is needed. The ZIP uses that same @GRZ_Local/Addons layout.

Prevents fracture state and fracture notifications; an existing saved fracture
is cleared on reconnect. Disables native walking damage to footwear and cuts
from walking barefoot or in ruined shoes. Existing wounds are not removed.
Bullet/melee health damage, combat bleeding, killfeed and hitmarker processing
are unchanged. This is not a fall-damage or general invulnerability mod; the
existing GRZ_Survival addon supplies the separately requested fall protection.

Implementation uses the current DayZ 1.30 SetBrokenLegs/ProcessFeetDamageServer
and BrokenLegsMdfr callbacks, preserving their original parameter names. PBO
and source checks do not compile Enforce or run DayZ. Test reconnecting a
fractured character and running barefoot on rough ground; verify normal
weapon damage still applies. Unsigned development PBO; sign with your own key
if your server enforces signatures.
