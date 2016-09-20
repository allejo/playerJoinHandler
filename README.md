# Player Join Handler

[![GitHub release](https://img.shields.io/github/release/allejo/playerJoinHandler.svg?maxAge=2592000)](https://github.com/allejo/playerJoinHandler/releases/latest)
![Minimum BZFlag Version](https://img.shields.io/badge/BZFlag-v2.4.0+-blue.svg)

A BZFlag plug-in that keeps track of player sessions and automatically moves players to the observer team if a match is in progress and they were not already on the server.

A session exists the entire time a player is on the server plus 120 seconds (this can be configured through `_sessionTime`) after they have disconnected. If a match is in progress, players will only be able join while their session is still active. By default, unverified players cannot join a match even if they have an session; this can be disabled by settings `_allowUnverified` to true.

## Usage

**Loading the plug-in**

This plug-in does not take any command line parameters; everything is configured through BZDB variables.

```
-loadplugin playerJoinHandler
```

**Custom BZDB Variables**

These custom BZDB variables must be used with `-setforced`, which sets BZDB variable `<name>` to `<value>`, even if the variable does not exist.

```
-setforced <name> <value>
```

| Name               | Type | Default | Description |
| ------------------ | ---- | ------- | ----------- |
| `_sessionTime`     | int  | 120     | The amount of seconds a player has to move from observer to a tank, a session |
| `_allowUnverified` | bool | false   | When set to true, unregistered players will be able to join a match if they already have a session |

## License

MIT
