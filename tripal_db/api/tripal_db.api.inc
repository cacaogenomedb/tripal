<?php

/**
 * @file
 * Provides an application programming interface (API) to manage references to external databases
 *
 * @defgroup tripal_db_api DB Module API
 * @ingroup tripal_api
 */

/**
 * Purpose: To retrieve a chado database object
 *
 * @param $select_values
 *   An array meant to uniquely select a given database
 *
 * @return
 *   Chado database object
 *
 * The database is selected using tripal_core_chado select and as such the
 * $select_values array parameter meant to uniquely identify the database to be
 * returned follows the same form as when using tripal_core_chado_select directly.
 *
 * Example Usage:
 * @code
    $select_values = array(
      'name' => 'SOFP'
    );
    $db_object = tripal_db_get_db($select_values);
 * @endcode
 *  The above code selects the SOFP db and returns the following object:
 * @code
    $db_object = stdClass Object (
      [db_id] => 49
      [name] => SOFP
      [description] =>
      [urlprefix] =>
      [url] =>
    );
 * @endcode
 *
 * @ingroup tripal_db_api
 */
function tripal_db_get_db($select_values) {

  $columns = array(
    'db_id',
    'name',
    'description',
    'urlprefix',
    'url'
  );
  $results = tripal_core_chado_select('db', $columns, $select_values);
  if (sizeof($results) == 1) {
    return $results[0];
  }
  elseif (empty($results)) {
    watchdog('tripal_cdb',
      'tripal_db_get_db: No db matches criteria values:%values',
      array('%values' => print_r($select_values, TRUE)),
      WATCHDOG_WARNING
    );
    return FALSE;
  }
  else {
    watchdog('tripal_db',
      'tripal_db_get_db: 2+ dbs match criteria values:%values',
      array('%values' => print_r($select_values, TRUE)),
      WATCHDOG_WARNING
    );
  }

}

/**
 * Purpose: To retrieve a chado db object
 *
 * @param $db_id
 *   db.db_id
 * @return
 *   Chado db object with all fields from the chado db table
 *
 * @ingroup tripal_db_api
 */
function tripal_db_get_db_by_db_id($db_id) {

  $r = db_fetch_object(chado_query(
    "SELECT * FROM {db} WHERE db_id=%d", $db_id
  ));

  return $r;
}

/**
 * Purpose: To retrieve a chado db object
 *
 * @param $name
 *   db.name
 * @return
 *   chado db object with all fields from the chado db table
 *
 * @ingroup tripal_db_api
 */
function tripal_db_get_db_by_name($name) {

  $values = array('name' => $name);
  $options = array('statement_name' => 'sel_db_na');
  
  $db = tripal_core_chado_select('db', array('*'), $values, $options);
  if (count($db) == 1) {
    return $db[0];
  }
  if (count($db) == 0) {
    return FALSE;
  }
  if (count($db) > 1) {
    return FALSE;
  }
}

// Purpose: To retrieve a chado db object
//
// @params where_options: array(
//                          <column_name> => array(
//                            'type' => <type of column: INT/**STRING>,
//                            'value' => <the vlaue you want to filter on>,
//                            'exact' => <if TRUE use =; if FALSE use ~>,
//                          )
//        )
// @return chado db object with all fields from the chado db table
//
//function tripal_db_get_db ($where_options) {

/**
 * Purpose: Create an options array to be used in a form element
 *   which provides a list of all chado dbs
 *
 * @return
 *   An array(db_id => name) for each db in the chado db table
 *
 * @ingroup tripal_db_api
 */
function tripal_db_get_db_options() {

  $result = chado_query(
    "SELECT db_id, name FROM {db}"
  );

  $options = array();
  while ( $r = db_fetch_object($result) ) {
    $options[$r->db_id] = $r->name;
  }

  return $options;

}

// Purpose: To retrieve a chado dbxref object
//
// @param where_options: array(
//                          <column_name> => array(
//                            'type' => <type of column: INT/**STRING>,
//                            'value' => <the vlaue you want to filter on>,
//                            'exact' => <if TRUE use =; if FALSE use ~>,
//                          )
//        )
// @return chado dbxref object with all fields from the chado dbxref table
//
//function tripal_db_get_dbxref ($where_options) {

/**
 * Purpose: To retrieve a chado database reference object
 *
 * @param $select_values
 *   An array meant to uniquely select a given database reference
 *
 * @return
 *   Chado database reference object
 *
 * The database reference is selected using tripal_core_chado select and as such the
 * $select_values array parameter meant to uniquely identify the database reference to be
 * returned follows the same form as when using tripal_core_chado_select directly.
 *
 * Example Usage:
 * @code
    $select_values = array(
      'accession' => 'synonym',
      'db_id' => array(
        'name' => 'SOFP'
      )
    );
    $dbxref_object = tripal_db_get_dbxref($select_values);
 * @endcode
 *  The above code selects the synonym database reference and returns the following object:
 * @code
    $dbxref_object = stdClass Object (
      [dbxref_id] => 2581
      [accession] => synonym
      [description] =>
      [version] =>
      [db_db_id] => 49
      [db_name] => SOFP
      [db_description] =>
      [db_urlprefix] =>
      [db_url] =>
    );
 * @endcode
 *
 * @ingroup tripal_db_api
 */
function tripal_db_get_dbxref($select_values) {

  $columns = array(
    'dbxref_id',
    'db_id',
    'accession',
    'description',
    'version'
  );
  $results = tripal_core_chado_select('dbxref', $columns, $select_values);
  if (sizeof($results) == 1) {
    $dbxref = tripal_db_add_db_to_object(array('db_id' => $results[0]->db_id), $results[0], array());
    unset($dbxref->db_id);

    return $dbxref;
  }
  elseif (empty($results)) {
    watchdog('tripal_db',
      'tripal_db_get_dbxref: No dbxref matches criteria values:%values',
      array('%values' => print_r($select_values, TRUE)),
      WATCHDOG_WARNING
    );
    return FALSE;
  }
  else {
    watchdog('tripal_db',
      'tripal_db_get_dbxref: 2+ dbxrefs match criteria values:%values',
      array('%values' => print_r($select_values, TRUE)),
      WATCHDOG_WARNING
    );
  }

}

/**
 * Purpose: To retrieve a chado dbxref object with a given accession
 *
 * @param $accession
 *   dbxref.accession
 * @param $db_id
 *   dbxref.db_id
 * @return
 *   chado dbxref object with all fields from the chado dbxref table
 *
 * @ingroup tripal_db_api
 */
function tripal_db_get_dbxref_by_accession($accession, $db_id=0) {

  if (!empty($db_id)) {
    $r = db_fetch_object(chado_query(
      "SELECT * FROM {dbxref} WHERE accession='%s' AND db_id=%d",
      $accession, $db_id
    ));
  }
  else {
    $r = db_fetch_object(chado_query(
      "SELECT * FROM {dbxref} WHERE accession='%s'",
      $accession
    ));
  }

  return $r;
}

/**
 * Adds a new database to the Chado DB table and returns the DB object.
 *
 * @param $dbname
 *   The name of the database. This name is usually used as the prefix for
 *   CV term accessions
 * @param $description
 *   Optional. A description of the database.  By default no description is required.
 * @param $url
 *   Optional. The URL for the database
 * @param $urlprefix
 *   Optional. The URL that is to be used as a prefix when constructing a link to
 *   a database term
 * @param $update
 *   Optional. Set this to '1' to force an update of the database if it
 *   already exists. The default is to not update. If the database exists
 *   then nothing is added.
 *
 * @return
 *   An object populated with fields from the newly added database.  If the 
 *   database already exists it returns the values in the current entry.
 *
 * @ingroup tripal_db_api
 */
function tripal_db_add_db($dbname, $description = '', $url = '', 
  $urlprefix = '', $update = 0) {

  // build the values array for inserting/updating
  $ins_values = array(
    'name' => $dbname,
    'description' => $description,
    'url' => $url,
    'urlprefix' => $urlprefix
  );
  
  // get the database record if it already exists
  $sel_values = array('name' => $dbname);
  $sel_options = array('statement_name' => 'sel_db_na');
  $result = tripal_core_chado_select('db', array('*'), $sel_values, $sel_options);
  
  // if it does not exists then add it
  if (count($result) == 0) {    
    $ins_options = array('statement_name' => 'ins_db_nadeurur');   
    $success = tripal_core_chado_insert('db', $ins_values, $ins_options); 
    if (!$success) {
      watchdog('tripal_db', "Cannot create db '$dbname'.", NULL, WATCHDOG_WARNING);
      return 0;
    }
    $result = tripal_core_chado_select('db', array('*'), $sel_values, $sel_options);
  }
  // if it exists and update is enabled the do the update
  elseif ($update) {
    $upd_options = array('statement_name' => 'upd_db_nadeurur');
    $success = tripal_core_chado_update('db', $sel_values, $ins_values, $upd_options);
    if (!$success) {
      watchdog('tripal_db', "Cannot update db '$dbname'.", NULL, WATCHDOG_WARNING);
      return 0;
    }
    $result = tripal_core_chado_select('db', array('*'), $sel_values, $sel_options);
  }
  
  // return the database object
  return $result[0];

}
/**
 *
 * @ingroup tripal_db_api
 */
function tripal_db_add_dbxref($db_id, $accession, $version = '', $description = '') {

  $ins_values = array(
    'db_id'       => $db_id,
    'accession'   => $accession,
    'version'     => $version,
    'description' => $description
  );
    
  // check to see if the dbxref exists
  $sel_values = array(
    'db_id'     => $db_id,
    'accession' => $accession,
  );
  $sel_options = array('statement_name' => 'sel_dbxref_dbacve', 'is_duplicate' => 1);
  $result = tripal_core_chado_select('dbxref', array('*'), $sel_values, $sel_options);
  
  // if it doesn't already exist then add it
  if (!$result) {
    $ins_options = array('statement_name' => 'ins_dbxref_dbacvede');
    $success = tripal_core_chado_insert('dbxref', $ins_values, $ins_options);
    if (!$success) {
      watchdog('tripal_cv', "Failed to insert the dbxref record $accession", NULL, WATCHDOG_WARNING);
      return 0;
    }
    $result = tripal_core_chado_select('dbxref', array('*'), $sel_values, $sel_options);
  }
  
  return $result[0];
}
