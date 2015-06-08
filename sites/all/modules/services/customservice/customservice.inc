<?php
// customservice.inc
/**
* Callback for creating note resources.
*
* @param object $data
* @return object
*/
function _customservice_create($data) {
  global $user;
  if (isset($data) &&  is_array($data)) $data=(object) $data;
  unset($data->id);
  $data->uid = $user->uid;
  $data->created = time();
  $data->modified = time();

  if (!isset($data->subject)) {
    return services_error('Missing note attribute subject', 406);
  }

  if (!isset($data->note)) {
    return services_error('Missing note attribute note', 406);
  }

  customservice_write_note($data);
 
  return (object)array(
    'id' => $data->id,
    'uri' => services_resource_uri(array('note', $data->id)),
  );
}

// customservice.inc
/**
* Callback for updating note resources.
*
* @param int $id
* @param object $data
* @return object
*/
function _customservice_update($id, $data) {
  global $user;
  $note = customservice_get_note($id);

  unset($data->created);
  $data->id = $id;
  $data->uid = $note->uid;
  $data->modified = time();

  customservice_write_note($data);
 
  return (object)array(
    'id' => $id,
    'uri' => services_resource_uri(array('note', $id)),
  );
}
/**
* Callback for retrieving note resources.
*
* @param int $id
* @return object
*/
function _customservice_retrieve($id) {
  return customservice_get_note($id);
}

/**
* Callback for deleting note resources.
*
* @param int $id
* @return object
*/
function _customservice_delete($id) {
  customservice_delete_note($id);
 
  return (object)array(
    'id' => $id,
  );
}

function _customservice_index($page, $parameters) {
  global $user;

  $notes = array();
  $res = db_query('SELECT * FROM {note} WHERE uid=:uid ORDER BY modified DESC',
           array(':uid' => $user->uid));

  foreach ($res as $note) {
    $notes[] = $note;
  }

  return $notes;
}


/**
* Access callback for the note resource.
*
* @param string $op
*  The operation that's going to be performed.
* @param array $args
*  The arguments that will be passed to the callback.
* @return bool
*  Whether access is given or not.
*/

function _customservice_service_access() {
return TRUE;
}


function _show_all_notes() { 

$mesg ="Ur name is:".$var1;
echo $mesg;
die();
return $mesg;
}



?>