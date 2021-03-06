<?php

/**
 * @file
 * Contains functions for viewing and editing of Materialized Views 
 * on a Tripal website.
 */

/**
 * A template function which returns markup to display details for the current materialized view
 *
 * @param $mview_id
 *  The unique ID of the materialized view to render
 *
 * @ingroup tripal_core
 */
function tripal_mview_report($mview_id) {

  // get this mview details
  $sql = "SELECT * FROM {tripal_mviews} WHERE mview_id = %d";
  $mview = db_fetch_object(db_query($sql, $mview_id));
  
  $rows = array();

  // create a table with each row containig stats for
  // an individual job in the results set.
  $return_url = url("admin/tripal/mviews/");
  $output .= "<p><a href=\"$return_url\">Return to table of materialized views.</a></p>";
  $output .= "<p>Details for <b>$mview->name</b>:</p>";
  
  // build the URLs using the url function so we can handle installations where
  // clean URLs are or are not used
  $update_url = url("admin/tripal/mviews/action/update/$mview->mview_id");
  $delete_url = url("admin/tripal/mviews/action/delete/$mview->mview_id");
  $edit_url = url("admin/tripal/mviews/edit/$mview->mview_id");
  $rows[] = array('Actions', "<a href='$update_url'>Populate</a>, <a href='$edit_url'>Edit</a>,  <a href='$delete_url'>Delete</a>");
  
  if ($mview->last_update > 0) {    
    $update = format_date($mview->last_update);
  }
  else {
    $update = 'Not yet populated';
  }
  $rows[] = array('Last Update', $update);  
  if ($mview->name) {
    $rows[] = array('View Name', $mview->name);
  }
  if ($mview->modulename) {
    $rows[] = array('Module Name', $mview->modulename);    
  }
  if ($mview->mv_table) {
    $rows[] = array('Table Name', $mview->mv_table);
  }
  if ($mview->mv_specs) {
    $rows[] = array('Table Field Definitions', $mview->mv_specs);
  }
  if ($mview->query) {
    $rows[] = array('Query', "<pre>" . $mview->query . "</pre>");
  }
  if ($mview->indexed) {
    $rows[] = array('Indexed Fields', $mview->indexed);
  }
  if ($mview->special_index) {
    $rows[] = array('Special Indexed Fields', $mview->special_index);
  }
  if ($mview->mv_schema) {
    $rows[] = array('Drupal Schema API Definition', "<pre>" . $mview->mv_schema . "</pre>");
  }
  
  $table = theme_table(array(), $rows);
  $output .= $table;

  return $output;
}

/**
 * A template function to render a listing of all Materialized Views
 *
 * @ingroup tripal_core
 */
function tripal_mviews_report() {
  $header = array('', 'MView Name', 'Last Update', 'Status', 'Description', '');
  $rows = array();
  $mviews = db_query("SELECT * FROM {tripal_mviews} ORDER BY name");

  while ($mview = db_fetch_object($mviews)) {
    if ($mview->last_update > 0) {
      $update = format_date($mview->last_update);
    }
    else {
      $update = 'Not yet populated';
    }

    $rows[] = array(
      l(t('View'), "admin/tripal/mviews/report/$mview->mview_id") ." | ".
      l(t('Edit'), "admin/tripal/mviews/edit/$mview->mview_id") ." | ".
      l(t('Populate'), "admin/tripal/mviews/action/update/$mview->mview_id"),
      $mview->name,
      $update,
      $mview->status,
      $mview->comment,
      l(t('Delete'), "admin/tripal/mviews/action/delete/$mview->mview_id"),
    );
  }

  $rows[] = array(
    'data' => array(
      array('data' => l(t('Create a new materialized view.'), "admin/tripal/mviews/new"),
        'colspan' => 6),
    )
  );
  $page = '</p>' . t("Materialized Views (MViews) are custom tables populated with a defined SQL statement.  
    Because Chado is highly normalized and highly constrained it serves as a wonderful 
    data storage platform, but unfortunately some queries may be slow.  MViews alleviate slowness by aggregating data
    into tables that are more easy to query.  Use MViews to create tables for custom search pages or custom Tripal
    module development.") . '</p>';
  $page .= '<p><b>' . t("MViews behaves in the following way:") . '</b><ul>'.
           '<li>' . t("The SQL statement defined for an MVIEW will be used to populate the table") . '</li>' .
           '<li>' . t("Altering the table structure of an MView will cause the MView table to be dropped and recreated.  All records in the MView will be lost.") . '</li>' .
           '<li>' . t("Altering the query of an existing view will not change the MView table. No records will be lost. ") . '</li>' .
           '<li>' . t("Repopulating an MView that is already populated will result in replacement of all records.") . '</li>' .
           '<li>' . t("A database transaction will be used when populating MViews. Therefore replacement of records does not occur until the query completes.  Any search forms or pages dependent on the MView will continue to function.") . '</li>' .
           '</ul></p>';
  $page .= '<b>' . t("Existing MViews") . '</b>';
  $page .= theme('table', $header, $rows);
  return $page;
}

/**
 * A Form to Create/Edit a Materialized View
 *
 * @param $form_state
 *   The current state of the form (Form API)
 * @param $mview_id
 *   The unique ID of the Materialized View to Edit or NULL if creating a new materialized view
 *
 * @return
 *   A form array (Form API)
 *
 * @ingroup tripal_core
 */
function tripal_mviews_form(&$form_state = NULL, $mview_id = NULL) {

  if (!$mview_id) {
    $action = 'Add';
  }
  else {
    $action = 'Edit';
  }

  // set defaults for collapsed fieldsets
  $schema_collapsed = 0;
  $traditional_collapsed = 1;  
  
  
  // get this requested view
  if (strcmp($action, 'Edit')==0) {
    $sql = "SELECT * FROM {tripal_mviews} WHERE mview_id = %d ";
    $mview = db_fetch_object(db_query($sql, $mview_id));

    // set the default values.  If there is a value set in the
    // form_state then let's use that, otherwise, we'll pull
    // the values from the database
    $default_name = $form_state['values']['name'];
    $default_mv_table = $form_state['values']['mv_table'];
    $default_mv_specs = $form_state['values']['mv_specs'];
    $default_indexed = $form_state['values']['indexed'];
    $default_mvquery = $form_state['values']['mvquery'];
    $default_special_index = $form_state['values']['special_index'];
    $default_comment = $form_state['values']['comment'];
    $default_modulename = $form_state['values']['modulename'];

    if (!$default_name) {
      $default_name = $mview->name;
    }
    if (!$default_mv_table) {
      $default_mv_table = $mview->mv_table;
    }
    if (!$default_mv_specs) {
      $default_mv_specs = $mview->mv_specs;
    }
    if (!$default_indexed) {
      $default_indexed = $mview->indexed;
    }
    if (!$default_mvquery) {
      $default_mvquery = $mview->query;
    }
    if (!$default_special_index) {
      $default_special_index = $mview->special_index;
    }
    if (!$default_comment) {
      $default_comment = $mview->comment;
    }
    if (!$default_schema) {
      $default_schema = $mview->mv_schema;
    }
    if (!$default_modulename) {      
      $default_modulename = $mview->modulename ? $mview->modulename : 'tripal_core';
    }
    

    // the mv_table column of the tripal_mviews table always has the table
    // name even if it is a custom table. However, for the sake of the form,
    // we do not want this to show up as the mv_table is needed for the
    // traditional style input.  We'll blank it out if we have a custom
    // table and it will get reset in the submit function using the
    // 'table' value from the schema array
    if ($default_schema) {
      $default_mv_table = '';
    }
    // set which fieldset is collapsed
    if (!$default_schema) {
      $schema_collapsed = 1;
      $traditional_collapsed = 0;
    }
  }

  // Build the form
  $form['action'] = array(
    '#type' => 'value',
    '#value' => $action
  );

  $form['mview_id'] = array(
    '#type' => 'value',
    '#value' => $mview_id
  );
  
  $form['modulename'] = array(
    '#type' => 'value',
    '#value' => $default_modulename,
  );

  $form['name']= array(
    '#type'          => 'textfield',
    '#title'         => t('View Name'),
    '#description'   => t('Please enter the name for this materialized view.'),
    '#required'      => TRUE,
    '#default_value' => $default_name,
  );

  $form['comment']= array(
    '#type'          => 'textarea',
    '#title'         => t('MView Description'),
    '#description'   => t('Optional.  Please provide a description of the purpose for this materialized vieww.'),
    '#required'      => FALSE,
    '#default_value' => $default_comment,
  );

  // add a fieldset for the Drupal Schema API
  $form['schema'] = array(
    '#type' => 'fieldset',
    '#title' => 'Drupal Schema API Setup',
    '#description' => t('Use the Drupal Schema API array to describe a table. The benefit is that it '.
                       'can be fully integrated with Tripal Views.  Tripal supports an extended '.
                       'array format to allow for descriptoin of foreign key relationships.'),
    '#collapsible' => 1,
    '#collapsed' => $schema_collapsed ,
  );

  $form['schema']['schema']= array(
    '#type'          => 'textarea',
    '#title'         => t('Schema Array'),
    '#description'   => t('Please enter the Drupal Schema API compatible array that defines the table.'),
    '#required'      => FALSE,
    '#default_value' => $default_schema,
    '#rows'          => 25,
  );

  // add a fieldset for the Original Table Description fields
  $form['traditional'] = array(
    '#type' => 'fieldset',
    '#title' => 'Legacy MViews Setup',
    '#description' => t('Traditionally MViews were created by specifying PostgreSQL style '.
                       'column types.  This method can be used but is deprecated in favor of the '.
                       'newer Drupal schema API method provided above. In rare cases where the Drupal Schema API ' .
                       'does not support a desired data type the Legacy Mviews should be used'),
    '#collapsible' => 1,
    '#collapsed' => $traditional_collapsed,
  );

  $form['traditional']['mv_table']= array(
    '#type'          => 'textfield',
    '#title'         => t('Table Name'),
    '#description'   => t('Please enter the table name that this view will generate in the database.  You can use the schema and table name for querying the view'),
    '#required'      => FALSE,
    '#default_value' => $default_mv_table,
  );

  $form['traditional']['mv_specs']= array(
    '#type'          => 'textarea',
    '#title'         => t('Table Definition'),
    '#description'   => t('Please enter the field definitions for this view. Each field should be separated by a comma or enter each field definition on each line.'),
    '#required'      => FALSE,
    '#default_value' => $default_mv_specs,
  );

  $form['traditional']['indexed']= array(
    '#type'          => 'textarea',
    '#title'         => t('Indexed Fields'),
    '#description'   => t('Please enter the field names (as provided in the table definition above) that will be indexed for this view.  Separate by a comma or enter each field on a new line.'),
    '#required'      => FALSE,
    '#default_value' => $default_indexed,
  );

  /**
  $form['traditional']['special_index']= array(
    '#type'          => 'textarea',
    '#title'         => t('View Name'),
    '#description'   => t('Please enter the name for this materialized view.'),
    '#required'      => TRUE,
    '#default_value' => $default_special_index,
  );
  */

  $form['mvquery']= array(
    '#type'          => 'textarea',
    '#title'         => t('Query'),
    '#description'   => t('Please enter the SQL statement used to populate the table.'),
    '#required'      => TRUE,
    '#default_value' => $default_mvquery,
    '#rows'          => 25,
  );

  if ($action == 'Edit') {
    $value = 'Save';
  }
  if ($action == 'Add') {
    $value = 'Add';
  }
  $form['submit'] = array(
    '#type'         => 'submit',
    '#value'        => t($value),
    '#weight'       => 9,
    '#executes_submit_callback' => TRUE,
  );
  $form['#redirect'] = 'admin/tripal/mviews';

  return $form;
}

/**
 * Validate the Create/Edit Materialized View Form
 * Implements hook_form_validate().
 *
 * @ingroup tripal_core
 */
function tripal_mviews_form_validate($form, &$form_state) {
  $action = $form_state['values']['action'];
  $mview_id = $form_state['values']['mview_id'];
  $name = $form_state['values']['name'];
  $mv_table = $form_state['values']['mv_table'];
  $mv_specs = $form_state['values']['mv_specs'];
  $indexed = $form_state['values']['indexed'];
  $query = $form_state['values']['mvquery'];
  $special_index = $form_state['values']['special_index'];
  $comment = $form_state['values']['comment'];
  $schema = $form_state['values']['schema'];

  if ($schema and ($mv_table or $mv_specs or $indexed or $special_index)) {
    form_set_error($form_state['values']['schema'],
      t('You can create an MView using the Drupal Schema API method or the '.
        'traditional method but not both.'));
  }
  if (!$schema) {
    if (!$mv_specs) {
      form_set_error($form_state['values']['mv_specs'],
        t('The Table Definition field is required.'));
    }
    if (!$mv_table) {
      form_set_error($form_state['values']['mv_table'],
        t('The Table Name field is required.'));
    }
  }

  // make sure the array is valid
  if ($schema) {
    $success = eval("\$schema_array = $schema;");
    if ($success === FALSE) {
      $error = error_get_last();
      form_set_error($form_state['values']['schema'],
        t("The schema array is improperly formatted. Parse Error : " . $error["message"]));
    }
    if (!array_key_exists('table', $schema_array)) {
      form_set_error($form_state['values']['schema'],
        t("The schema array must have key named 'table'"));
    }

    // TODO: add in more validation checks of the array to help the user
  }
}

/**
 * Submit the Create/Edit Materialized View Form
 * Implements hook_form_submit().
 *
 * @ingroup tripal_core
 */
function tripal_mviews_form_submit($form, &$form_state) {

  $ret = array();
  $action = $form_state['values']['action'];
  $mview_id = $form_state['values']['mview_id'];
  $name = $form_state['values']['name'];
  $mv_table = $form_state['values']['mv_table'];
  $mv_specs = $form_state['values']['mv_specs'];
  $indexed = $form_state['values']['indexed'];
  $query = $form_state['values']['mvquery'];
  $special_index = $form_state['values']['special_index'];
  $comment = $form_state['values']['comment'];
  $schema = $form_state['values']['schema'];
  $modulename = $form_state['values']['modulename'];
  if (!$modulename) {
    $modulename = 'tripal_core';
  }

  if (strcmp($action, 'Edit') == 0) {
    tripal_edit_mview($mview_id, $name, $modulename, $mv_table, $mv_specs,
      $indexed, $query, $special_index, $comment, $schema);
  }
  elseif (strcmp($action, 'Add') == 0) {
    tripal_add_mview($name, $modulename, $mv_table, $mv_specs,
      $indexed, $query, $special_index, $comment, $schema);
  }
  else {
    drupal_set_message(t("No action performed."));
  }

  return '';
}