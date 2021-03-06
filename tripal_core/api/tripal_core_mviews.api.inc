<?php
/**
 * @file
 * Contains functions for the Materialized Views API

 * @defgroup tripal_mviews_api Materalized Views API
 * @ingroup tripal_core_api
 * @{
 * Provides an application programming interface (API) to manage materialized views in Chado.
 * The Perl-based chado comes with an interface for managing materialzed views.  This
 * API provides an alternative Drupal-based method.
 * @}
 */

/**
 * Add a materialized view to the chado database to help speed data access. This
 * function supports the older style where postgres column specifications
 * are provided using the $mv_table, $mv_specs and $indexed variables. It also
 * supports the newer preferred method where the materialized view is described
 * using the Drupal Schema API array.
 *
 * @param $name
 *   The name of the materialized view.
 * @param $modulename
 *   The name of the module submitting the materialized view (e.g. 'tripal_library')
 * @param $mv_table
 *   The name of the table to add to chado. This is the table that can be queried.
 * @param $mv_specs
 *   The table definition
 * @param $indexed
 *   The columns that are to be indexed
 * @param $query
 *   The SQL query that loads the materialized view with data
 * @param $special_index
 *   currently not used
 * @param $comment
 *   A string containing a description of the materialized view
 * @param $mv_schema
 *   If using the newer Schema API array to define the materialized view then
 *   this variable should contain the array or a string representation of the 
 *   array.
 * 
 * @ingroup tripal_mviews_api
 */
function tripal_add_mview($name, $modulename, $mv_table, $mv_specs, $indexed,
  $query, $special_index, $comment = NULL, $mv_schema = NULL) {

  // get the table name from the schema array
  $schema_arr = array();
  if ($mv_schema) {
    // if the schema is provided as a string then convert it to an array
    if (!is_array($mv_schema)) {
      eval("\$schema_arr = $mv_schema;");
    }
    // if the schema is provided as an array then create a string
    // copy of it for storage in the mview 
    else {
      $schema_arr = $mv_schema;
      $mv_schema = var_export($schema_arr, 1);
    }
    $mv_table = $schema_arr['table'];
  }

  // Create a new record
  $record = new stdClass();
  $record->name = $name;
  $record->modulename = $modulename;
  $record->mv_table = $mv_table;
  $record->mv_specs = $mv_specs;
  $record->indexed = $indexed;
  $record->query = $query;
  $record->special_index = $special_index;
  $record->comment = $comment;
  $record->mv_schema = $mv_schema;

  // add the record to the tripal_mviews table and if successful
  // create the new materialized view in the chado schema
  if (drupal_write_record('tripal_mviews', $record)) {

    // drop the table from chado if it exists
    $previous_db = tripal_db_set_active('chado');  // use chado database
    if (db_table_exists($mv_table)) {
      $sql = "DROP TABLE $mv_table";
      db_query($sql);
    }
    tripal_db_set_active($previous_db);  // now use drupal database

    // now construct the indexes
    $index = '';
    if ($indexed) {
      // add to the array of values
      $vals = preg_split("/[\n,]+/", $indexed);
      $index = '';
      foreach ($vals as $field) {
        $field = trim($field);
        $index .= "CREATE INDEX idx_${mv_table}_${field} ON $mv_table ($field);";
      }
    }

    // create the table differently depending on if it the traditional method
    // or the Drupal Schema API method
    if ($mv_schema) {
      if (!tripal_core_create_custom_table ($ret, $mv_table, $schema_arr, 0)) {
        drupal_set_message(t("Could not create the materialized view. Check Drupal error report logs."), 'error');
      }
      else {
        drupal_set_message(t("View '%name' created", array('%name' => $name)));
      }
    }
    else {
      // add the table to the database
      $sql = "CREATE TABLE {$mv_table} ($mv_specs); $index";
      $previous_db = tripal_db_set_active('chado');  // use chado database
      $results = db_query($sql);
      tripal_db_set_active($previous_db);  // now use drupal database
      if ($results) {
        drupal_set_message(t("View '%name' created", array('%name' => $name)));
      }
      else {
        drupal_set_message(t("Failed to create the materialized view table: '%mv_table'", array('%mv_table' => $mv_table)), 'error');
      }
    }
  }
}

/**
 * Edits a materialized view to the chado database to help speed data access.This
 * function supports the older style where postgres column specifications
 * are provided using the $mv_table, $mv_specs and $indexed variables. It also
 * supports the newer preferred method where the materialized view is described
 * using the Drupal Schema API array.
 *
 * @param $mview_id
 *   The mview_id of the materialized view to edit
 * @param $name
 *   The name of the materialized view.
 * @param $modulename
 *   The name of the module submitting the materialized view (e.g. 'tripal_library')
 * @param $mv_table
 *   The name of the table to add to chado. This is the table that can be queried.
 * @param $mv_specs
 *   The table definition
 * @param $indexed
 *   The columns that are to be indexed
 * @param $query
 *   The SQL query that loads the materialized view with data
 * @param $special_index
 *   currently not used
 * @param $comment
 *   A string containing a description of the materialized view
 * @param $mv_schema
 *   If using the newer Schema API array to define the materialized view then
 *   this variable should contain the array.
 *
 * @ingroup tripal_mviews_api
 */
function tripal_edit_mview($mview_id, $name, $modulename, $mv_table, $mv_specs,
  $indexed, $query, $special_index, $comment = NULL, $mv_schema = NULL) {

  // get the table name from the schema array
  $schema_arr = array();
  if ($mv_schema) {
    // get the schema from the mv_specs and use it to add the custom table
    eval("\$schema_arr = $mv_schema;");
    $mv_table = $schema_arr['table'];
  }

  $record = new stdClass();
  $record->mview_id = $mview_id;
  $record->name = $name;
  $record->modulename = $modulename;
  $record->query = $query;
  $record->last_update = 0;
  $record->status = '';
  $record->comment = $comment;

  // get the view before we update and check to see if the table structure has
  // changed. IF so, then we want to drop and recreate the table. If not, then
  // just save the updated SQL.
  $create_table = 1;  
  $sql = "SELECT * FROM {tripal_mviews} WHERE mview_id = %d";
  $mview = db_fetch_object(db_query($sql, $mview_id));  
  if($mview->mv_schema == $mv_schema and $mview->mv_table == $mv_table and 
     $mview->mv_specs == $mv_specs and $mview->indexed == $indexed and 
     $mview->special_index == $special_index) {
    // nothing has changed so simpy update the SQL and other fields 
    $create_table = 0;
  }
  else {
    // add in the table structure fields
    $record->mv_schema = $mv_schema;
    $record->mv_table = $mv_table;
    $record->mv_specs = $mv_specs;
    $record->indexed = $indexed;
    $record->query = $query;
    $record->special_index = $special_index;
  }
  
  // if we are going to create the table then we must first drop it if it exists
  if ($create_table) {
    $previous_db = tripal_db_set_active('chado');  // use chado database
    if (db_table_exists($mview->mv_table)) {
      $sql = "DROP TABLE %s";
      db_query($sql, $mview->mv_table);
      drupal_set_message(t("View '%name' dropped", array('%name' => $name)));
    }
    tripal_db_set_active($previous_db);  // now use drupal database
  }

  // update the record to the tripal_mviews table and if successful
  // create the new materialized view in the chado schema
  if (drupal_write_record('tripal_mviews', $record, 'mview_id')) {
    // construct the indexes SQL if needed
    $index = '';
    if ($indexed) {
      // add to the array of values
      $vals = preg_split("/[\n,]+/", $indexed);
      $index = '';
      foreach ($vals as $field) {
        $field = trim($field);
        $index .= "CREATE INDEX idx_${mv_table}_${field} ON $mv_table ($field);";
      }
    }

    // re-create the table differently depending on if it the traditional method
    // or the Drupal Schema API method
    if ($create_table and $mv_schema) {     
      if (!tripal_core_create_custom_table($ret, $mv_table, $schema_arr, 0)) {
        drupal_set_message(t("Could not create the materialized view. Check Drupal error report logs."));
      }
      else {
        drupal_set_message(t("View '%name' created", array('%name' => $name)));
      }
    }
    if ($create_table and !$mv_schema) {
      $sql = "CREATE TABLE {$mv_table} ($mv_specs); $index";
      $results = chado_query($sql);
      if ($results) {
        drupal_set_message(t("View '%name' created.  All records cleared. Please re-populate the view.", 
          array('%name' => $name)));
      }
      else {
        drupal_set_message(t("Failed to create the materialized view table: '%mv_table'", 
          array('%mv_table' => $mv_table)), 'error');
      }
    }
    if (!$create_table) {
      $message = "View '%name' updated.  All records remain. ";
      if ($query != $mview->query) {
        $message .= "Please repopulate the view to use updated query.";
      }
      drupal_set_message(t($message, array('%name' => $name)));  
    }
  }
  else {
    drupal_set_message(t("Failed to update the materialized view: '%mv_table'", 
      array('%mv_table' => $mv_table)), 'error');  
  }
}

/**
 * Retrieve the materialized view_id given the name
 *
 * @param $view_name
 *   The name of the materialized view
 *
 * @return
 *   The unique identifier for the given view
 *
 * @ingroup tripal_mviews_api
 */
function tripal_mviews_get_mview_id($view_name) {
  $sql = "SELECT * FROM {tripal_mviews} ".
        "WHERE name = '%s'";
  if (db_table_exists('tripal_mviews')) {
    $mview = db_fetch_object(db_query($sql, $view_name));
    if ($mview) {
      return $mview->mview_id;
    }
  }

  return FALSE;
}

/**
 * Does the specified action for the specified Materialized View
 *
 * @param $op
 *   The action to be taken. One of update or delete
 * @param $mview_id
 *   The unique ID of the materialized view for the action to be performed on
 * @param $redirect
 *   TRUE/FALSE depending on whether you want to redirect the user to admin/tripal/mviews
 *
 * @ingroup tripal_core
 */
function tripal_mviews_action($op, $mview_id, $redirect = FALSE) {
  global $user;

  $args = array("$mview_id");
  if (!$mview_id) {
    return '';
  }

  // get this mview details
  $sql = "SELECT * FROM {tripal_mviews} WHERE mview_id = %d";
  $mview = db_fetch_object(db_query($sql, $mview_id));

  // add a job or perform the action based on the given operation
  if ($op == 'update') {
    tripal_add_job("Populate materialized view '$mview->name'", 'tripal_core',
       'tripal_update_mview', $args, $user->uid);
  }
  if ($op == 'delete') {
    // remove the mview from the tripal_mviews table
    $sql = "DELETE FROM {tripal_mviews} ".
           "WHERE mview_id = $mview_id";
    db_query($sql);
    // drop the table from chado if it exists
    $previous_db = tripal_db_set_active('chado');  // use chado database
    if (db_table_exists($mview->mv_table)) {
      $sql = "DROP TABLE $mview->mv_table";
      db_query($sql);
    }
    tripal_db_set_active($previous_db);  // now use drupal database
  }

  // Redirect the user
  if ($redirect) {
    drupal_goto("admin/tripal/mviews");
  }
}

/**
 * Update a Materialized View
 *
 * @param $mview_id
 *   The unique identifier for the materialized view to be updated
 *
 * @return
 *   True if successful, FALSE otherwise
 *
 * @ingroup tripal_mviews_api
 */
function tripal_update_mview($mview_id) {
  $sql = "SELECT * FROM {tripal_mviews} WHERE mview_id = %d ";
  $mview = db_fetch_object(db_query($sql, $mview_id));
  if ($mview) {    
    // execute the query inside a transaction so that it doesn't destroy existing data
    // that may leave parts of the site unfunctional
    tripal_db_start_transaction();
    $previous_db = tripal_db_set_active('chado');  // use chado database
    $results = db_query("DELETE FROM {%s}", $mview->mv_table);
    $results = db_query("INSERT INTO {%s} ($mview->query)", $mview->mv_table);    
    tripal_db_set_active($previous_db);  // now use drupal database
    if ($results) {
      // commit the transaction
      tripal_db_commit_transaction();
      $sql = "SELECT count(*) as cnt FROM {%s}";
      $previous_db = tripal_db_set_active('chado');  // use chado database
      $count = db_fetch_object(db_query($sql, $mview->mv_table));
      tripal_db_set_active($previous_db);  // now use drupal database
      $record = new stdClass();
      $record->mview_id = $mview_id;
      $record->last_update = time();
      $record->status = "Populated with " . number_format($count->cnt) . " rows";
      drupal_write_record('tripal_mviews', $record, 'mview_id');
      return TRUE;
    }
    else {
      // rollback the transaction
      tripal_db_rollback_transaction();
      // print and save the error message
      $record = new stdClass();
      $record->mview_id = $mview_id;
      $record->status = "ERROR populating. See Drupal's recent log entries for details.";
      print $record->status . "\n";
      drupal_write_record('tripal_mviews', $record, 'mview_id');
      return FALSE;
    }
  }
}