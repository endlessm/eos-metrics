/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2014, 2015 Endless Mobile, Inc. */

/* This file is part of eos-metrics.
 *
 * eos-metrics is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * eos-metrics is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with eos-metrics.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef EMTR_EVENT_TYPES_H
#define EMTR_EVENT_TYPES_H

#if !(defined(_EMTR_INSIDE_EOSMETRICS_H) || defined(COMPILING_EOS_METRICS))
#error "Please do not include this header file directly."
#endif

G_BEGIN_DECLS

/**
 * SECTION:emtr-event-types
 * @title: Event Types
 * @short_description: shared constant definitions for event types
 *
 * Event types are RFC 4122 UUIDs. This file provides a mapping from
 * 36-character string representations of UUIDs to human-readable constants.
 * UUIDs should never be recycled since this will create confusion when
 * analyzing the metrics database. The list is sorted alphabetically by name.
 *
 * To generate a new UUID on Endless OS, Debian, or Ubuntu:
 * |[
 * sudo apt-get install uuid-runtime
 * uuidgen -r
 * ]|
 */

#ifndef EMTR_DISABLE_DEPRECATED

EMTR_DEPRECATED_IN_0_4
gboolean emtr_event_id_to_name (const gchar  *event_id,
                                const gchar **event_name);

/**
 * EMTR_EVENT_USER_IS_LOGGED_IN:
 *
 * Started when a user logs in and stopped when that user logs out.
 *
 * Deprecated: 0.2: A newer version of this event type is defined in
 * src/eos-metrics-instrumentation.c in the eos-metrics-instrumentation repo.
 */
#define EMTR_EVENT_USER_IS_LOGGED_IN "ab839fd2-a927-456c-8c18-f1136722666b"

/**
 * EMTR_EVENT_NETWORK_STATUS_CHANGED:
 *
 * Recorded when the network changes from one of the states described at
 * https://developer.gnome.org/NetworkManager/unstable/spec.html#type-NM_STATE
 * to another. The auxiliary payload is a 2-tuple of the form
 * (previous_network_state, new_network_state). Since events are delivered on a
 * best-effort basis, there is no guarantee that the new network state of the
 * previous successfully recorded network status change event matches the
 * previous network state of the current network status change event.
 *
 * Deprecated: 0.4: This event type is now defined in
 * src/eos-metrics-instrumentation.c in the eos-metrics-instrumentation repo.
 */
#define EMTR_EVENT_NETWORK_STATUS_CHANGED "5fae6179-e108-4962-83be-c909259c0584"

/**
 * EMTR_EVENT_SHELL_APP_IS_OPEN:
 *
 * Occurs when an application visible to the shell is opened or closed. The payload
 * varies depending on whether it is given as an opening event or a closed event.
 * If it is an opening event, the payload is a human-readable application name.
 * If it is a closing event, the payload is empty. The key used is a pointer to the
 * corresponding ShellApp.
 *
 * Deprecated: 0.4: This event type is now defined in src/shell-app-system.c in
 * the eos-desktop repo.
 */
#define EMTR_EVENT_SHELL_APP_IS_OPEN "b5e11a3d-13f8-4219-84fd-c9ba0bf3d1f0"

/**
 * EMTR_EVENT_SOCIAL_BAR_IS_VISIBLE:
 *
 * This is started when the social bar is visible and stopped when hidden.
 *
 * Deprecated: 0.4: This event type is now defined in EosSocial/socialBar.js in
 * the eos-social repo.
 */
#define EMTR_EVENT_SOCIAL_BAR_IS_VISIBLE "9c33a734-7ed8-4348-9e39-3c27f4dc2e62"

/**
 * EMTR_EVENT_SHELL_APP_ADDED:
 *
 * Occurs when an application is installed, aka is added to the desktop's app
 * grid.
 *
 * Deprecated: 0.4: This event type is now defined in js/ui/shellDBus.js in the
 * eos-desktop repo.
 */
#define EMTR_EVENT_SHELL_APP_ADDED "51640a4e-79aa-47ac-b7e2-d3106a06e129"

/**
 * EMTR_EVENT_SHELL_APP_REMOVED:
 *
 * Occurs when an application is uninstalled, aka is removed from the desktop's
 * app grid. This can happen via uninstalling in the app store or
 * dragging / dropping an app to the trash.
 *
 * Deprecated: 0.4: This event type is now defined in js/ui/iconGridLayout.js in
 * the eos-desktop repo.
 */
#define EMTR_EVENT_SHELL_APP_REMOVED "683b40a7-cac0-4f9a-994c-4b274693a0a0"

#endif /* EMTR_DISABLE_DEPRECATED */

G_END_DECLS

#endif /* EMTR_EVENT_TYPES_H */
