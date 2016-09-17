# Player Join Handler

[![GitHub release](https://img.shields.io/github/release/allejo/playerJoinHandler.svg?maxAge=2592000)](https://github.com/allejo/playerJoinHandler/releases/latest)
![Minimum BZFlag Version](https://img.shields.io/badge/BZFlag-v2.4.0+-blue.svg)

A BZFlag plug-in that keeps track of player sessions and automatically moves players to the observer team if a match is in progress and they didn't already have a session.

## Usage

**Loading the plug-in**

```
-loadplugin playerJoinHandler
```

**Available BZDB Variables**

These custom BZDB variables must be used with `-setforced`.

- `_sessionTime` - (int) The amount of seconds a player has to move from observer to a tank, a session
- `_allowUnverified` - (bool) When set to true, unregistered players will be able to join a match if they already have a session

## License

MIT
