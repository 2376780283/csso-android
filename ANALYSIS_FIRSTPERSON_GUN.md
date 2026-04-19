# CS:GO 第一人称视图下半身武器渲染问题分析

## 问题描述

在第一人称视图中，玩家屏幕上看到两把枪：

1. **Viewmodel（视觉模型）** - 第一人称专用的手臂和武器模型
   - 位置：屏幕右下角（通常）
   - 用途：为本地玩家展示第一人称视角
   - 特点：这是唯一应该在第一人称显示的武器

2. **Worldmodel（世界模型）** - 玩家模型骨骼上的武器
   - 位置：玩家下半身模型上（如果下半身被强制显示）
   - 原因：当强制显示本地玩家的下半身时，由于没有正确屏蔽上半身，附着在背上或手上的武器也被渲染了
   - 问题：这会造成双枪渲染的视觉错误

## 代码分析

### 关键代码位置

文件：`game/client/cstrike/c_cs_player.cpp`

#### 1. 第一人称模型可见性控制 (第2269-2276行)
```cpp
void C_CSPlayer::UpdateAddonModels( bool bForce )
{
    int iCurAddonBits = m_iAddonBits;

    // Don't put addon models on the local player unless in third person.
    if ( IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer() )
        iCurAddonBits = 0;

    // If the local player is observing this entity in first-person mode, get rid of its addons.
    C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
    if ( pPlayer && pPlayer->GetObserverMode() == OBS_MODE_IN_EYE && pPlayer->GetObserverTarget() == this )
        iCurAddonBits = 0;
```

**说明：** 这里控制addon models（附件模型）的显示，在第一人称时设置为0

#### 2. ShouldDraw() 函数控制渲染 (第3560-3590行)
```cpp
bool C_CSPlayer::ShouldDraw( void )
{
    // If we're dead, our ragdoll will be drawn for us instead.
    if ( !IsAlive() )
        return false;

    if( GetTeamNumber() == TEAM_SPECTATOR )
        return false;

    if( IsLocalPlayer() )
    {
        if ( IsRagdoll() )
            return true;
    }

    C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

    // keep drawing players we're observing with the interpolating spectator camera
    if ( pLocalPlayer && pLocalPlayer->GetObserverInterpState() == OBSERVER_INTERP_TRAVELING )
    {
        return true;
    }

    // don't draw players we're observing in first-person
    if ( pLocalPlayer && pLocalPlayer->GetObserverTarget() == ToBasePlayer(this) && pLocalPlayer->GetObserverMode() == OBS_MODE_IN_EYE )
    {
        return false;
    }

    return BaseClass::ShouldDraw();
}
```

**说明：** 控制玩家主体模型的可见性。在第一人称时（OBS_MODE_IN_EYE），不绘制玩家身体。

#### 3. 本地玩家绘制控制 (第3866-3872行)
```cpp
void C_CSPlayer::DoExtraBoneProcessing( CStudioHdr *pStudioHdr, Vector pos[], Quaternion q[], matrix3x4_t boneToWorld[], CBoneBitList &boneComputed, CIKContext *pIKContext )
{
    if ( !m_bUseNewAnimstate || !m_PlayerAnimStateCSGO )
        return;
    
    if ( !IsVisible() || (IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer()) || !ShouldDraw() )
        return;
```

**说明：** 在处理骨骼时检查本地玩家是否应该被绘制

#### 4. Addon Models 管理 (第2265-2340行)
```cpp
void C_CSPlayer::UpdateAddonModels( bool bForce )
{
    int iCurAddonBits = m_iAddonBits;

    // Don't put addon models on the local player unless in third person.
    if ( IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer() )
        iCurAddonBits = 0;

    // If the local player is observing this entity in first-person mode, get rid of its addons.
    C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
    if ( pPlayer && pPlayer->GetObserverMode() == OBS_MODE_IN_EYE && pPlayer->GetObserverTarget() == this )
        iCurAddonBits = 0;
```

## 解决方案

### 问题根源

当强制显示本地玩家的身体模型时（例如通过某些mod或调试选项），weaponworldmodel 仍然会被渲染，因为：

1. 本地玩家的下半身被渲染
2. 武器世界模型附着在玩家的骨骼上
3. 没有特殊的检查来屏蔽第一人称视图下的武器世界模型

### 推荐修复方案

#### 方案 1：在 Worldmodel 的 ShouldDraw 中添加检查

在 weapon worldmodel 的渲染函数中添加以下检查：

```cpp
// 在武器世界模型的 ShouldDraw() 或类似函数中
if ( IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer() )
{
    // 本地玩家在第一人称视图中不应该显示武器世界模型
    return false;
}
```

#### 方案 2：在 UpdateAddonModels 中处理武器世界模型

扩展 `UpdateAddonModels` 函数来也隐藏武器世界模型：

```cpp
void C_CSPlayer::UpdateAddonModels( bool bForce )
{
    int iCurAddonBits = m_iAddonBits;

    // Don't put addon models on the local player unless in third person.
    if ( IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer() )
        iCurAddonBits = 0;

    // 新增：隐藏本地玩家第一人称视图中的武器世界模型
    CWeaponCSBase *pWeapon = GetActiveCSWeapon();
    if ( pWeapon && IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer() )
    {
        CBaseWeaponWorldModel *pWeaponWorldModel = pWeapon->m_hWeaponWorldModel.Get();
        if ( pWeaponWorldModel )
        {
            pWeaponWorldModel->AddEffects( EF_NODRAW );
        }
    }
    // ... 其余代码
}
```

#### 方案 3：在武器获取/生成时添加标志

在武器世界模型初始化时为本地玩家的武器添加隐藏标志：

```cpp
// 当武器绑定到本地玩家时
if ( pOwner && pOwner->IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer() )
{
    pWeaponWorldModel->AddEffects( EF_NODRAW );
}
```

## 关键类和结构

- `C_CSPlayer`: 客户端玩家实体
- `C_BaseWeaponWorldModel`: 武器世界模型
- `CWeaponCSBase`: CS:GO武器基类
- `m_iAddonBits`: 控制插件模型（如手套、武器等）的显示位
- `C_BasePlayer::ShouldDrawLocalPlayer()`: 确定本地玩家是否应该被绘制

## 文件位置

- 主文件：`game/client/cstrike/c_cs_player.cpp`
- 头文件：`game/client/cstrike/c_cs_player.h`
- 相关文件：
  - `game/client/cstrike/c_baseviewmodel.cpp` (viewmodel处理)
  - `game/shared/weapon_basecsgloves.h` (手套/addon处理)

## 建议

要彻底解决这个问题，应该：

1. **立即修复**：在 `UpdateAddonModels()` 中添加武器世界模型的隐藏逻辑
2. **长期方案**：创建一个通用的"第一人称模型隐藏"系统，统一管理所有不应该在第一人称显示的模型
3. **测试**：确保在各种场景下测试（第一人称、第三人称、观看其他玩家等）

