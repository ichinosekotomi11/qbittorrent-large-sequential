# qBittorrent Large Torrent Sequential Download

基于 [qBittorrent](https://github.com/qbittorrent/qBittorrent) 的非官方修改版，提供大型 Torrent 内部按文件顺序下载、95% 提前切换，以及移动端 WebUI 样式。

## 功能

- 功能默认关闭，默认阈值为 **50 GiB**，可在 WebUI 设置中修改。
- 按每个 Torrent 的总大小独立判断；小于阈值的 Torrent 使用原有行为。
- 按 metadata 文件索引推进，当前文件达到 **95%** 时开始下一个文件。
- 在当前调度阶段内，已启动且未完成的前序文件和当前文件使用最高优先级；后续等待文件设为 DND。
- 保留用户 DND 与原始优先级的内存记录；关闭功能时恢复原优先级。
- 不修改 libtorrent，也不依靠 Torrent-level Sequential Download 实现。

95% 提前切换会使多个未完成文件同时下载。跨文件 Piece 也可能产生少量后续文件数据；本功能不能保证只占用一集的磁盘空间。

## Docker

需要 Docker Engine 和 Docker Compose 插件。从源码本地构建：

```sh
docker compose -f docker-compose.qbittorrent.yml up -d --build
```

访问 `http://localhost:8080`，查看启动日志获取首次登录信息：

```sh
docker compose -f docker-compose.qbittorrent.yml logs qbittorrent
```

仓库不提供预置账号密码。首次启动由 qBittorrent 生成登录凭据，请登录后自行设置密码。

默认使用通用目录：

| 宿主机目录 | 容器目录 | 用途 |
| --- | --- | --- |
| `./docker-config` | `/config` | 首次运行生成的独立配置与任务状态 |
| `./docker-downloads` | `/downloads` | 下载数据 |

默认映射 WebUI 端口 `8080` 和 BT 端口 `6881/tcp+udp`。请在客户端设置中确认监听端口为 `6881`。部署到公网前自行设置身份认证、防火墙或反向代理。

Dockerfile 使用原构建环境 Ubuntu 25.04；该发行版已结束支持，文件已配置归档软件源以便构建。长期部署建议迁移并验证受支持的构建环境。

## WebAPI 配置

使用 qBittorrent 现有 `/api/v2/app/preferences` 和 `/api/v2/app/setPreferences`，新增字段为：

```json
{
  "large_torrent_sequential_download": false,
  "large_torrent_sequential_threshold": 53687091200
}
```

阈值单位为字节。

## 状态与已知限制

当前串行管理器的原始优先级保存在进程内存中，尚未实现独立持久化。重启时会读取 qBittorrent 恢复的优先级，因此临时 DND 与用户 DND 的区别在重启后不能保证保留。不要把本版本视为已经通过所有重启、强制校验与外部自动删任务场景的完整验收。

已在原部署环境完成增量编译与 WebUI 启动检查；脱敏副本未宣称完成全新 Docker 构建或真实 80 GiB 下载验收。

## 隐私与发布范围

本仓库为源码快照，使用全新 Git 历史。仅包含通用源码、构建文件及说明，不包含个人运行配置、下载任务、tracker passkey、服务器地址、部署凭据、日志或本地构建产物。原作者版权和许可证声明完整保留。

源码基线：上游提交 `9cc9aeade3b87b659a4ec22b9033fe3878207a06`。上游说明见 [README.upstream.md](README.upstream.md)。

## 许可证

遵循 qBittorrent 原有许可证及第三方组件许可证，详见 [COPYING](COPYING)、[AUTHORS](AUTHORS) 与源码声明。本仓库不代表 qBittorrent 官方发行。
