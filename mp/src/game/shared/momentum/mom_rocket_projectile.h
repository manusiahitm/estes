#pragma once

#ifndef CLIENT_DLL
#include "smoke_trail.h"
#endif

#define MOM_ROCKET_RADIUS (110.0f * 1.1f) // https://github.com/NicknineTheEagle/TF2-Base/blob/master/src/game/shared/tf/tf_weaponbase_rocket.h#L27
#define MOM_ROCKET_SPEED 1100

#ifndef CLIENT_DLL
class CMomentumRocketLauncher;

class CMomentumRocket : public CBaseAnimating
{
    DECLARE_CLASS(CMomentumRocket, CBaseAnimating);

  public:
    CMomentumRocket();

    void Spawn();
    void Precache();
    void Touch(CBaseEntity *pOther);
    void Explode(trace_t *pTrace, CBaseEntity *pOther);

    virtual float GetRadius() { return MOM_ROCKET_RADIUS; }
    virtual float GetDamage() { return m_flDamage; }
    virtual void SetDamage(float flDamage) { m_flDamage = flDamage; }

    CHandle<CMomentumRocketLauncher> m_hOwner;

    static CMomentumRocket *EmitRocket(const Vector &vecSrc, const QAngle &vecAngles, CBaseEntity *pentOwner);

  protected:
    void CreateSmokeTrail();

    CHandle<RocketTrail> m_hRocketTrail;
    float m_flDamage;

  private:
    DECLARE_DATADESC();
};
#endif