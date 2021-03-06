=========
Changelog
=========

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog <https://keepachangelog.com/en/1.0.0/>`_, except reStructuredText is used instead of Markdown.
Please use only reStructuredText in this file, no Markdown!

This project does not currently adhere to semantic versioning.

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
