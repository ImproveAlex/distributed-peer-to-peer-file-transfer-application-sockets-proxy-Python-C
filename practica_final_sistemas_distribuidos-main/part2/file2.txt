#ifndef implementation_H
#define implementation_H

#include <stdio.h>
#include <string.h>
#include "common.h"

// Register function
enum RC register_user(char *user);

// Unregister function
enum RC unregister_user( char *user);

// Connect function
enum RC connect_user(char *user_name, char *ip, int port);

// Disconnect function
enum RC disconnect_user(char *user);

// Publish function
enum RC publish(char *user_name, char *fileName, char *description);

// Delete function
enum RC delete_file(char *user_name, char *fileName);

// List users function
enum RC list_users(char *user_name, USER_CONNECTED **users_connected_for_listing, int *users_connected_for_listing_count);

// List content function
enum RC list_content(char *user_name, char *user_for_content_request, Publication **publications_for_listing, int *publications_for_listing_count);


#endif /* implementation_H */
