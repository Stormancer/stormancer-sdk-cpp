=========
Changelog
=========

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog <https://keepachangelog.com/en/1.0.0/>`_, except reStructuredText is used instead of Markdown.
Please use only reStructuredText in this file, no Markdown!

This project does not currently adhere to semantic versioning.

Unreleased
----------

Added
*****

- ``Users.hpp``: Header-only replacement for the Authentication plugin (``AuthenticationService`` and the like).
- ``Party.hpp``: Header-only replacement for the Party plugin.
- Added test code for ``Users.hpp`` and ``Party.hpp``.

Changed
*******

- ``AuthenticationService::sendRequestToUser()``: Breaking change to the data contract, requires the latest server-side Users plugin.

Fixed
*****

- ``AuthenticationService::sendRequestToUser()``: OriginId now has a correct value on the recipient's side.
- Server-to-client RPCs now complete properly on the server when the client completes them without sending any data. Previously, they would hang until the client disconnected from the scene.
- Fixed rare unobserved exceptions in ``ApiClient`` and ``RPC`` (prevents crashes in some cases, especially that could be triggered by breakpoints).
