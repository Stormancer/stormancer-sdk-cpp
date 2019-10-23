=========
Changelog
=========

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog <https://keepachangelog.com/en/1.0.0/>`_, except reStructuredText is used instead of Markdown.
Please use only reStructuredText in this file, no Markdown!

This project does not currently adhere to semantic versioning.

Unreleased
----------

Changed
*******

- All use of Peer Id in the client library has been replaced with Session Id. This is required for Stormancer server 2.9 and up. Note that compatibility with 2.8 is broken as a result.
  If you need 2.8 compatibility, please use v1.8_.
- *Party* Plugin:

  * Updates to the party state, made by calling methods like ``PartyApi::updatePlayerStatus()``, are now immediately applied locally, rather than after the server responds.
    The local state can auto-correct if there is a divergence with the server.
  * ``PartyApi::updatePlayerStatus()`` now retries automatically (and indefinitely) upon receiving a ``party.settingsOutdated`` error.

Fixed
*****

- Disconnect From Scene system requests issued from server to client are now properly handled.
- System Request error messages sent from the server are now properly forwarded up to the calling client code across all platforms.
- Prevent a rare crash in RakNetTransport during client disconnection.

.. _v1.8:

v1.8
----

Added
*****

- ``Users.hpp``: Header-only replacement for the Authentication plugin (``AuthenticationService`` and the like).
- Users plugin: New *credentials renewal* feature. Client credentials can now have an expiration date, upon which the server can ask the client for new credentials.
- ``Party.hpp``: Header-only replacement for the Party plugin.
- Added test code for ``Users.hpp`` and ``Party.hpp``.
- ``Steam.hpp``: Header-only Steam plugin.

Changed
*******

- The API for the Users (former Authentication) and Party plugins have some name changes.
- ``Users::UsersApi::sendRequestToUser()``: Breaking change to the data contract, requires the latest server-side Users plugin.
- All plugings use the ``Users`` plugin instead of the ``Authentication`` plugin.
- When a System Request returns an error, ``(msgId:<type id of the request>)`` is no longer appended to the exception's message.
- Incoming packets are now processed sequentially. As a result, RakNet packet ordering (``RELIABLE_ORDERED``, ``RELIABLE_SEQUENCED``) is now respected with P2P connections. Note that ordering is still not guaranteed with server connections.

Fixed
*****

- ``Users::UsersApi::sendRequestToUser()``: OriginId now has a correct value on the recipient's side.
- Server-to-client RPCs now complete properly on the server when the client completes them without sending any data. Previously, they would hang until the client disconnected from the scene.
- Fixed rare unobserved exceptions in ``ApiClient`` and ``RPC`` (prevents crashes in some cases, especially that could be triggered by breakpoints).
- Various other stability improvements.

Removed
*******

- Deprecated ``Authentication`` plugin has been removed.
