# 修复方案：隐藏第一人称下半身的世界模型武器

## 问题总结

在CS:GO中，当玩家处于第一人称视图时，会出现两把枪的现象：
- **Viewmodel**（视觉模型）：屏幕上显示的手臂和武器，用于第一人称
- **Worldmodel**（世界模型）：附着在玩家模型骨骼上的武器，当下半身被强制显示时也会被渲染

这会导致视觉混乱。

## 根本原因

在 `c_cs_player.cpp` 中：

1. `UpdateAddonModels()` 函数（第2265行）正确地隐藏了addon models
2. 但**没有**隐藏武器的世界模型（weapon worldmodel）
3. 当下半身模型被渲染时，附着在骨骼上的武器也会被一起渲染

## 推荐修复步骤

### 修复方案：在 UpdateAddonModels 中添加 Worldmodel 隐藏逻辑

#### 位置：`game/client/cstrike/c_cs_player.cpp`，`UpdateAddonModels()` 函数（第2265行附近）

#### 修改前的代码（第2265-2280行）：
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

    if ( bForce )
        iCurAddonBits = 0;
```

#### 修改后的代码（建议添加内容）：
在第2276行（if语句后）添加以下代码来隐藏weapon worldmodel：

```cpp
    // Hide weapon worldmodel for local player in first-person view
    // This prevents the double-gun rendering issue when the lower body is forced to be visible
    if ( IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer() )
    {
        CWeaponCSBase *pWeapon = GetActiveCSWeapon();
        if ( pWeapon )
        {
            CBaseWeaponWorldModel *pWeaponWorldModel = pWeapon->m_hWeaponWorldModel.Get();
            if ( pWeaponWorldModel && !pWeaponWorldModel->IsEffectActive( EF_NODRAW ) )
            {
                pWeaponWorldModel->AddEffects( EF_NODRAW );
            }
        }
    }
    else
    {
        // Re-enable weapon worldmodel when switching to third-person or for non-local players
        CWeaponCSBase *pWeapon = GetActiveCSWeapon();
        if ( pWeapon )
        {
            CBaseWeaponWorldModel *pWeaponWorldModel = pWeapon->m_hWeaponWorldModel.Get();
            if ( pWeaponWorldModel && pWeaponWorldModel->IsEffectActive( EF_NODRAW ) )
            {
                pWeaponWorldModel->RemoveEffects( EF_NODRAW );
            }
        }
    }
```

## 完整修改代码块

将以下代码插入 `c_cs_player.cpp` 的 `UpdateAddonModels()` 函数中，位置在第 2276 行之后：

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

    if ( bForce )
        iCurAddonBits = 0;

    // [NEW CODE START] Hide weapon worldmodel for local player in first-person view
    // This prevents the double-gun rendering issue
    CWeaponCSBase *pWeapon = GetActiveCSWeapon();
    if ( pWeapon )
    {
        CBaseWeaponWorldModel *pWeaponWorldModel = pWeapon->m_hWeaponWorldModel.Get();
        
        if ( IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer() )
        {
            // Hide in first-person
            if ( pWeaponWorldModel && !pWeaponWorldModel->IsEffectActive( EF_NODRAW ) )
            {
                pWeaponWorldModel->AddEffects( EF_NODRAW );
            }
        }
        else
        {
            // Show in third-person or for other players
            if ( pWeaponWorldModel && pWeaponWorldModel->IsEffectActive( EF_NODRAW ) )
            {
                pWeaponWorldModel->RemoveEffects( EF_NODRAW );
            }
        }
    }
    // [NEW CODE END]

    // Rest of the existing code continues below...
    // Any changes to the attachments we should have?
    if ( !bForce &&
        m_iLastAddonBits == iCurAddonBits &&
        m_iLastPrimaryAddon == m_iPrimaryAddon &&
        m_iLastSecondaryAddon == m_iSecondaryAddon &&
        m_iLastKnifeAddon == m_iKnifeAddon )
    {
        return;
    }
    // ... rest of function
}
```

## 替代方案：在武器类中处理

如果上述方案不可行，也可以在武器世界模型的类中添加第一人称检查（在weaponworldmodel相关代码中）：

```cpp
// In weapon worldmodel's ShouldDraw() or similar function
bool C_BaseWeaponWorldModel::ShouldDraw()
{
    C_BasePlayer *pOwner = ToBasePlayer( GetOwnerEntity() );
    
    // Hide weapon worldmodel when local player is in first-person view
    if ( pOwner && pOwner->IsLocalPlayer() && !C_BasePlayer::ShouldDrawLocalPlayer() )
        return false;
    
    return BaseClass::ShouldDraw();
}
```

## 测试清单

修改后应该测试以下场景：

- [ ] 第一人称视图中只显示viewmodel武器，没有worldmodel武器
- [ ] 第三人称视图中正确显示武器worldmodel
- [ ] 切换视角时武器正确显示/隐藏
- [ ] 观看其他玩家时武器正确显示
- [ ] 观看第一人称的队友时（in-eye观察）没有看到双枪
- [ ] 武器更换时正确处理
- [ ] 死亡后再生时正确处理

## 相关函数和变量

| 项目 | 说明 |
|------|------|
| `IsLocalPlayer()` | 检查是否为本地玩家 |
| `C_BasePlayer::ShouldDrawLocalPlayer()` | 检查本地玩家是否应被绘制（第三人称模式） |
| `GetActiveCSWeapon()` | 获取当前武器 |
| `CBaseWeaponWorldModel::AddEffects(EF_NODRAW)` | 添加不绘制效果 |
| `CBaseWeaponWorldModel::RemoveEffects(EF_NODRAW)` | 移除不绘制效果 |
| `IsEffectActive()` | 检查是否有特定效果 |

## 代码注释和解释

```
EF_NODRAW: Entity Flag - 不绘制此实体
    - 当设置此标志时，实体将不会被渲染
    - 物理和逻辑仍然正常工作，只是不显示

IsLocalPlayer(): 
    - 如果这是本地（控制的）玩家返回true
    - 用于区分本地玩家和其他玩家

ShouldDrawLocalPlayer():
    - 返回本地玩家是否应该被绘制
    - 在第一人称时返回false
    - 在第三人称或观察模式时返回true
```

## 可能的副作用和处理

1. **武器附件未隐藏**：确保addon models也被正确隐藏
   - 解决：这已经在原代码中处理（iCurAddonBits = 0）

2. **切换视角时武器闪烁**：
   - 原因：可能是timing问题
   - 解决：添加检查确保效果被正确应用/移除

3. **其他玩家的武器错误隐藏**：
   - 原因：检查不够精确
   - 解决：确保只对IsLocalPlayer()为true时应用隐藏

## 验证修改

编译后可以通过以下方式验证：

```
1. 启动游戏
2. 进入第一人称（默认）
3. 观察屏幕 - 应该只看到一把枪（viewmodel）
4. 如果启用了显示下半身的mod/console命令
5. 验证worldmodel被隐藏，没有出现双枪
6. 按V或其他按键进入第三人称
7. 验证武器worldmodel现在可见
8. 换回第一人称
9. 验证武器worldmodel再次隐藏
```

## 总结

这个修复通过：
1. 检测本地玩家在第一人称模式下
2. 隐藏武器的世界模型（AddEffects EF_NODRAW）
3. 在第三人称或其他情况下显示它
4. 防止了viewmodel和worldmodel同时显示的问题

修改最小，不会影响其他游戏玩法。
