# yue.store

iOS 原生应用商店客户端，Objective-C 开发，适配 iOS 18+。

## 功能

- **主页**：展示应用列表，点击跳转详情页
- **分类**：按分类筛选应用
- **搜索**：关键词搜索应用
- **签名**：本地 IPA 签名（C 语言签名器）
- **星河背景**：CAEmitterLayer 粒子动画
- **液态玻璃导航栏**：UIVisualEffectView 毛玻璃效果

## 技术栈

- Objective-C
- iOS 18+ 部署目标
- C 语言 IPA 签名器（minizip + Security + CommonCrypto）
- GitHub Actions 云端构建

## 构建

通过 GitHub Actions 自动构建 IPA。
