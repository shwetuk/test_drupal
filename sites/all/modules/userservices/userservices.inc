<?php
function services_access_menu() {
  return TRUE;
}
function _userservices__services_resources() {
  $definition = array(
    'user' => array(
	 'operations' => array(
	 ),
      
      'actions' => array(
	     'test' => array(
          'help' => 'test a user for a new session',
          'callback' => '_userservices_test',
          'args' => array(
            array(
              'name' => 'var1',
              'type' => 'string',
              'description' => 'A valid username',
              'source' => array('data' => 'username'),
              'optional' => FALSE,
            ),
            array(
              'name' => 'var2',
              'type' => 'string',
              'description' => 'A valid password',
              'source' => array('data' => 'password'),
              'optional' => FALSE,
            ),
          ),
          'access callback' => 'services_access_menu',
          'file' => array('type' => 'module', 'module' => 'services', 'name' => 'userservices'),
        ),
       
      ),
      'targeted_actions' => array(
        'cancel' => array(
          'help' => 'Cancel a user',
          'callback' => '_userservice_cancel',
          'file' => array('type' => 'module', 'module' => 'services', 'name' => 'userservices'),
          'access callback' => '_userservices_access',
          'access arguments' => array('cancel'),
          'access arguments append' => TRUE,
          'args' => array(
            array(
              'name' => 'uid',
              'optional' => FALSE,
              'source' => array('path' => 0),
              'type' => 'int',
              'description' => 'The id of the user to cancel.',
            ),
          ),
        ),
        'password_reset' => array(
          'access callback' => '_userservices_access',
          'access arguments' => array('password_reset'),
          'access arguments append' => TRUE,
          'callback' => '_userservice_password_reset',
          'args' => array(
            array(
              'name' => 'uid',
              'optional' => FALSE,
              'source' => array('path' => 0),
              'type' => 'int',
              'description' => 'The id of the user whose password to reset.',
            ),
          ),
        ),
        'resend_welcome_email' => array(
          'access callback' => '_userservices_access',
          'access arguments' => array('resend_welcome_email'),
          'access arguments append' => TRUE,
          'callback' => '_userservice_resend_welcome_email',
          'args' => array(
            array(
              'name' => 'uid',
              'optional' => FALSE,
              'source' => array('path' => 0),
              'type' => 'int',
              'description' => 'The id of the user whose welcome email to resend.',
            ),
          ),
        ),
      ),
    ),
  );

  $definition['user']['actions']['register'] = array_merge($definition['user']['operations']['create'], array(
    'help' => 'Register a user',
  ));

  return $definition;
}

function _userservices_test($var1, $var2) {


$message = "var1 is ".$var1."var2 is ".$var2;
return $message;
}

/**
 * Login a user using the specified credentials.
 *
 * Note this will transfer a plaintext password.
 *
 * @param $username
 *   Username to be logged in.
 * @param $password
 *   Password, must be plain text and not hashed.
 *
 * @return
 *   A valid session object.
 */
function _userservices_login($username, $password) {
  global $user;

  if ($user->uid) {
    // user is already logged in
    return services_error(t('Already logged in as @user.', array('@user' => $user->name)), 406);
  }

  // Check if account is active.
  if (user_is_blocked($username)) {
    return services_error(t('The username %name has not been activated or is blocked.', array('%name' => $username)), 403);
  }

  // Emulate drupal native flood control: check for flood condition.
  $flood_state = array();
  if (variable_get('services_flood_control_enabled', TRUE)) {
    $flood_state = _user_resource_flood_control_precheck($username);
  }

  // Only authenticate if a flood condition was not detected.
  if (empty($flood_state['flood_control_triggered'])) {
    $uid = user_authenticate($username, $password);
  }
  else {
    $uid = FALSE;
  }

  // Emulate drupal native flood control: register flood event, and throw error
  // if a flood condition was previously detected
  if (variable_get('services_flood_control_enabled', TRUE)) {
    $flood_state['uid'] = $uid;
    _user_resource_flood_control_postcheck($flood_state);
  }

  if ($uid) {
    $user = user_load($uid);
    if ($user->uid) {
      user_login_finalize();

      $return = new stdClass();
      $return->sessid = session_id();
      $return->session_name = session_name();
      $return->token = drupal_get_token('services');

      $account = clone $user;

      services_remove_user_data($account);

      $return->user = $account;

      return $return;
    }
  }
  watchdog('user', 'Invalid login attempt for %username.', array('%username' => $username));
  return services_error(t('Wrong username or password.'), 401);
}

/**
 * Logout the current user.
 */
function _userservices_logout() {
  global $user;

  if (!$user->uid) {
    // User is not logged in
    return services_error(t('User is not logged in.'), 406);
  }

  watchdog('user', 'Session closed for %name.', array('%name' => $user->name));

  // Destroy the current session.
  module_invoke_all('user_logout', $user);
  session_destroy();

  // Load the anonymous user.
  $user = drupal_anonymous_user();

  return TRUE;
}
