<?php

include('views/handlers/views_handler_join_chado_through_linking.inc');
include('views/handlers/views_handler_join_chado_aggregator.inc');
include('api/tripal_views.api.inc');


/**
 * @file
 * Tripal Views Integration
 *
 * @defgroup views Views Integration
 * @{
 * Provide rules for formatting and composition of fields
 * @}
 *
 * @defgroup views_handlers Views Integration Handlers
 * @ingroup views
 * @{
 * Provide rules for formatting and composition of fields
 * @}
 *
 * @defgroup views_field_handlers Views Field Handlers
 * @ingroup views_handlers
 * @{
 * Provide rules for formatting and composition of fields
 * @}
 *
 *
 * @defgroup views_filter_handlers Views Filter Handlers
 * @ingroup views_handlers
 * @{
 * Provide the ability to filter based on specified data
 * @}
 *
 * @defgroup views_sort_handlers Views Sort Handlers
 * @ingroup views_handlers
 * @{
 * Provide methods describing how specific data should be sorted
 * @}
 *
 * @defgroup views_argument_handlers Views Arguement Handlers
 * @ingroup views_handlers
 * @{
 * Provide the ability to filter pased on arguments in the path of the view
 * @}
 */

/**
 * Implements hook_views_handlers()
 *
 * Purpose: Register all custom handlers with views
 *   where a handler describes either "the type of field",
 *   "how a field should be filtered", "how a field should be sorted"
 *
 * @return
 *   An array of handler definitions
 *
 * @ingroup tripal_views
 */
function tripal_views_views_handlers() {

  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'tripal_views') . '/views/handlers',
    ),
    'handlers' => array(

      // Custom Tripal Filter Handlers
      'tripal_views_handler_filter_file_upload' => array(
        'parent' => 'views_handler_filter',
      ),
      'tripal_views_handler_filter_textarea' => array(
        'parent' => 'views_handler_filter',
      ),
      'tripal_views_handler_filter_no_results' => array(
        'parent' => 'views_handler_filter'
      ),
      'tripal_views_handler_filter_select_cvterm' => array(
        'parent' => 'views_handler_filter_string',
      ),
      'tripal_views_handler_filter_select_string' => array(
        'parent' => 'chado_views_handler_filter_string',
      ),
      'tripal_views_handler_filter_sequence' => array(
        'parent' => 'chado_views_handler_filter_string',
      ),

      // Custom Tripal Field Handlers
      'tripal_views_handler_field_aggregate' => array(
        'parent' => 'chado_views_handler_field',
      ),
      'tripal_views_handler_field_sequence' => array(
        'parent' => 'chado_views_handler_field',
      ),

      // Join Handlers
      'views_handler_join_chado_aggregator' => array(
        'parent' => 'views_join',
      ),
      'views_handler_join_chado_through_linking' => array(
        'parent' => 'views_join',
      ),

      // Relationship Handlers
      'chado_views_handler_relationship' => array(
        'parent' => 'views_handler_relationship'
      ),
      'chado_views_handler_relationship_to_node' => array(
        'parent' => 'views_handler_relationship'
      ),

      // Wrappers for Default Views Handlers-----
      // Field Handlers
      'chado_views_handler_field' => array(
        'parent' => 'views_handler_field'
      ),
      'chado_views_handler_field_boolean' => array(
        'parent' => 'views_handler_field_boolean'
      ),
      'chado_views_handler_field_counter' => array(
        'parent' => 'views_handler_field_counter'
      ),
      'chado_views_handler_field_custom' => array(
        'parent' => 'views_handler_field_custom'
      ),
      'chado_views_handler_field_date' => array(
        'parent' => 'views_handler_field_date'
      ),
      'chado_views_handler_field_markup' => array(
        'parent' => 'views_handler_field_markup'
      ),
      'chado_views_handler_field_math' => array(
        'parent' => 'views_handler_field_math'
      ),
      'chado_views_handler_field_numeric' => array(
        'parent' => 'views_handler_field_numeric'
      ),
      // Filter Handlers
      'chado_views_handler_filter_string' => array(
        'parent' => 'views_handler_filter_string',
      ),
      'chado_views_handler_filter_boolean_operator_string' => array(
        'parent' => 'views_handler_filter_boolean_operator_string',
      ),
      'chado_views_handler_filter_boolean_operator' => array(
        'parent' => 'views_handler_filter_boolean_operator',
      ),
      'chado_views_handler_filter_date' => array(
        'parent' => 'views_handler_filter_date',
      ),
      'chado_views_handler_filter_equality' => array(
        'parent' => 'views_handler_filter_equality',
      ),
      'chado_views_handler_filter_float' => array(
        'parent' => 'views_handler_filter_float',
      ),
      'chado_views_handler_filter_numeric' => array(
        'parent' => 'views_handler_filter_numeric',
      ),
      // Sort Handlers
      'chado_views_handler_sort' => array(
        'parent' => 'views_handler_sort'
      ),
      'chado_views_handler_sort_date' => array(
        'parent' => 'views_handler_sort_date'
      ),
      'chado_views_handler_sort_formula' => array(
        'parent' => 'views_handler_sort_formula'
      ),
      'chado_views_handler_sort_menu_hierarchy' => array(
        'parent' => 'views_handler_sort_menu_hierarchy'
      ),
      'chado_views_handler_sort_random' => array(
        'parent' => 'views_handler_sort_random'
      ),
    ),
  );
}

/**
 * Implements hook_views_pre_render
 *
 * Purpose: Intercepts the view after the query has been executed
 *   All the results are stored in $view->result
 *   Looking up the NID here ensures the query is only executed once
 *   for all stocks in the table.
 *
 * @todo add if !<chado/drupal same db> around NID portion
 *
 * @ingroup tripal_views
 */
function tripal_views_views_pre_render(&$view) {

  // We need to unset the exposed_input for the view so we can repopulate that
  // variable. This is necessary if we're using the file_upload_combo
  // custom form element which adds the file_path variable to the $_GET after the
  // view has populated the $view->exposed_input variable
  unset($view->exposed_input);

  // we want to add to the bottom of the views the form for downloading
  // results in other formats (e.g. Excel, FASTA, CSV, etc.).  The Views Data
  // Export module provides small images at the bottom, but we want to provide
  // a more intutitive interface for getting different file formats
  $form = drupal_get_form('tripal_views_data_export_download_form', $view, $display_id, $args);
  $view->attachment_after = $form;
}

/**
 * Generates a dynamic data array for Views
 *
 * Purpose: This function is a hook used by the Views module. It populates and
 *   returns a data array that specifies for the Views module the base table,
 *   the tables it joins with and handlers.  The data array is populated
 *   using the data stored in the tripal_views tables.
 *
 * @return a data array formatted for the Views module
 *
 * @ingroup tripal_views
 */

function tripal_views_views_data() {

  // Make sure all chado tables are integrated
  tripal_views_integrate_all_chado_tables();

  // Define Global Fields
  // Filter handler that lets the admin say:
  // "Show no results until they enter search parameters"
  $data['views']['search_results'] = array(
    'title' => t('Search Results'),
    'help' => t('Delay results until Apply/Search is clicked by the user.'),
    'filter' => array(
      'handler' => 'tripal_views_handler_filter_no_results',
    ),
  );

  $tvi_query = db_query('SELECT * FROM {tripal_views}');

  while ($tvi_row = db_fetch_object($tvi_query)) {

    // check to see if this is the lightest (drupal-style) priority setup for this table
    // if not then don't use this definition
    $lightest_priority_setup = tripal_views_is_lightest_priority_setup($tvi_row->setup_id, $tvi_row->table_name);
    if (!$lightest_priority_setup) {
      continue;
    }

    // ids we'll use for queries
    $setup_id = $tvi_row->setup_id;
    $mview_id = $tvi_row->mview_id;

    // holds the base table name and fields
    $base_table = '';
    $base_fields = array();

    // indicated whether the current table is a base table or not
    $is_base_table = $tvi_row->base_table;

    // populate the base table name and fields.  If an $mview_id is given
    // then get the materialized view info, otherwise get the Chado table info
    if ($mview_id) {

      // get the base table name from the materialized view
      $sql = "SELECT name, mv_specs FROM {tripal_mviews} WHERE mview_id = %d";
      $mview_table = db_fetch_object(db_query($sql, $mview_id));
      $base_table = $mview_table->name;

      // get the columns in this materialized view.  They are separated by commas
      // where the first word is the column name and the rest is the type
      $columns = explode(",", $mview_table->mv_specs);
      foreach ($columns as $column) {
        $column = trim($column);  // trim trailing and leading spaces
        preg_match("/^(.*?)\ (.*?)$/", $column, $matches);
        $column_name = $matches[1];
        $column_type = $matches[2];
        $base_fields[$column_name] = array(
          'column_name' => $column_name,
          'type' => $column_type,
        );
      }

      // get the field name and descriptions
      $sql = "SELECT * FROM {tripal_views_field} WHERE setup_id=%d";
      $query = db_query($sql, $setup_id);
      while ($field = db_fetch_object($query)) {
        $base_fields[$field->column_name]['name'] = $field->name;
        $base_fields[$field->column_name]['help'] = $field->description;
      }
    }
    // if this is not a legacy materialized view
    else {
      $base_table = $tvi_row->table_name;

      // Check if we are trying to integrate the node table
      // if we are we want to add to a current integration
      // as compared to create a whole new one
      if ($base_table == 'node') {

      	/* -- SPF Jun 13, 2013 -- not sure why the node table 
      	 *    is included in the Chado integrated tables. I've commented
      	 *    out.  I think it was here for integration of chado tables wiht 
      	 *    nodes, but code to do this has been added below and it's not
      	 *    necessary to have the node table in the list of integrated tables
      	 *
        // Add any joins between the node table and other tables
        $sql = "SELECT * FROM {tripal_views_join} WHERE setup_id = %d";
        $joins = db_query($sql, $setup_id);
        while ($join = db_fetch_object($joins)) {
          $left_table = $join->left_table;
          $left_field = $join->left_field;
          $base_field = $join->base_field;
          $handler = $join->handler;

          // add join entry
          $data[$base_table]['table']['join'][$left_table] = array(
            'left_field' => $left_field,
            'field' => $base_field,
          );
          if ($handler) {
            $data[$base_table]['table']['join'][$left_table]['handler'] = $handler;
          }
        }


        // Add in any handlers for node fields
        $sql = "SELECT * FROM {tripal_views_handlers} WHERE setup_id=%d";
        $query = db_query($sql, $setup_id);
        while ($handler = db_fetch_object($query)) {
          $data[$base_table][$handler->column_name][$handler->handler_type]['handler'] = $handler->handler_name;

          // Add in any additional arguments
          // This should be a serialized array including (at a minimum) name => <handler name>
          if ($handler->arguments) {
            $data[$base_table][$handler->column_name][$handler->handler_type] = array_merge($data[$base_table][$handler->column_name][$handler->handler_type], unserialize($handler->arguments));
          }
        }
        */
        continue;
      } 

      // get the table description
      $table_desc = tripal_core_get_chado_table_schema($base_table);

      $fields = $table_desc['fields'];
      if (!is_array($fields)) {
        $fields = array();
      }
      foreach ($fields as $column => $attrs) {
        $base_fields[$column] = array(
          'column_name' => $column,
          'type' => $attrs['type'],
        );
      }

      // get the field name and descriptions
      $sql = "SELECT * FROM {tripal_views_field} WHERE setup_id=%d";
      $query = db_query($sql, $setup_id);
      while ($field = db_fetch_object($query)) {
        $base_fields[$field->column_name]['name'] = $field->name;
        $base_fields[$field->column_name]['help'] = $field->description;
      }
    }

    // Setup the base table info in the data array
    $data[$base_table]['table']['group'] = t("$tvi_row->name");

    if ($is_base_table) {
      $data[$base_table]['table']['base'] = array(
        'group' => "$tvi_row->name",
        'title' => "$tvi_row->name",
        'help'  => $tvi_row->comment,
      );
    }
    else {
      $data[$base_table]['table'] = array(
        'group' => "$tvi_row->name",
        'title' => "$tvi_row->name",
        'help'  => $tvi_row->comment,
      );
    }

    // first add the fields
    foreach ($base_fields as $column_name => $base_field) {
      $data[$base_table][$column_name] = array(
        'title' => t($base_field['name']),
        'help' => t($base_field['help']),
        'field' => array(
          'click sortable' => TRUE,
        ),
      );


      // now add the handlers
      $sql = "SELECT * FROM {tripal_views_handlers} WHERE setup_id = %d AND column_name = '%s'";
      $handlers = db_query($sql, $setup_id, $column_name);
      while ($handler = db_fetch_object($handlers)) {
        $data[$base_table][$column_name][$handler->handler_type]['handler'] = $handler->handler_name;

        // Add in any additional arguments
        // This should be a serialized array including (at a minimum) name => <handler name>
        if ($handler->arguments) {
          $data[$base_table][$column_name][$handler->handler_type] = array_merge($data[$base_table][$column_name][$handler->handler_type], unserialize($handler->arguments));
        }
      };
      
      // add in joins to the node tables if the Chado schema is local
		  if (tripal_core_chado_schema_exists()) {
		    
		    $linker_table = 'chado_' . $base_table;
		    $linker_name = ucwords(str_replace('_', ' ', $base_table));
		    $schema = tripal_core_get_chado_table_schema($base_table);
		    $pkey = $schema['primary key'][0];
		    
		    // if a node linking table exists then add in the joins
		    if (db_table_exists($linker_table)) {		
		    		    		      
		    	// add the linker table to the data array
			    $data[$linker_table] = array(
			      // describe the table
			      'table' => array(
			        'title' => 'Chado ' . $linker_name,
              'group' => 'Chado ' . $linker_name,
			        // join to the base table and the node table
			        'join' => array(
				        $base_table => array(
			            'left_field' => $pkey,
			            'field' => $pkey,
				        ),
				        'node' => array(
                  'left_field' => 'nid',
                  'field' => 'nid',
				        ),
				      ),
				    ),
				    // add the nid field
            'nid' => array(
              'title' => t($linker_name . ' Node ID'),
              'help' => t("The node ID for this $linker_name"),
              'field' => array(
                'handler' => 'views_handler_field_numeric',
                'click sortable' => TRUE,
              ),
              'filter' => array(
                'handler' => 'views_handler_filter_numeric',
              ),
              'sort' => array(
                'handler' => 'views_handler_sort',
              ),
            ),
            // add the primary key field
            $pkey => array(
              'title' => t($linker_name . ' ' . $pkey),
              'help' => t("The $base_table primary key"),
              'field' => array(
                'handler' => 'views_handler_field_numeric',
                'click sortable' => TRUE,
              ),
              'filter' => array(
                'handler' => 'views_handler_filter_numeric',
              ),
              'sort' => array(
                'handler' => 'views_handler_sort',
              ),
            ),            
          );  

          // add the join instructions for the base table
          // (we'll add the recipricol joins for the node table in the hook_views_data_alter
          $data[$base_table]['table']['join'][$linker_table] = array(
            'left_field' => 'organism_id',
            'field' => 'organism_id',
          );
          $data[$base_table]['table']['join']['node'] = array(
            'left_table' => $linker_table,
            'left_field' => 'organism_id',
            'field' => 'organism_id',
          );

			
			    // Add relationship between linker and base table
			    $data[$linker_table][$base_table . '_nid'] = array(
			      'group' => $linker_name,
			      'title' => $linker_name . 'Node',
			      'help' => "Links Chado $linker_name Fields/Data to the Nodes in the current View.",
			      'real field' => $pkey,
			      'relationship' => array(
			        'handler' => 'views_handler_relationship',
			        'title' => t("Chado => $linker_name"),
			        'label' => t("Chado => $linker_name"),
			        'real field' => $pkey,
			        'base' => $base_table,
			        'base field' => $pkey
			      ),
			    );
			
			    // Add node relationship to base table
			    $data[$linker_table][$linker_table .'_nid'] = array(
			      'group' => $linker_name,
			      'title' => $linker_name . 'Node',
			      'help' => "Links Chado $linker_name Fields/Data to the Nodes in the current View.",
			      'real field' => 'nid',
			      'relationship' => array(
			        'handler' => 'views_handler_relationship',
			        'title' => t('Chado => Node'),
			        'label' => t('Chado => Node'),
			        'real field' => 'nid',
			        'base' => 'node',
			        'base field' => 'nid'
			      ),
			    );
		    } 
		  }		  
    }

    // now add the joins
    $sql = "SELECT * FROM {tripal_views_join} WHERE setup_id = %d";
    $joins = db_query($sql, $setup_id);
    if (!isset($joins)) {
      $joins = array();
    }
    while ($join = db_fetch_object($joins)) {
      $left_table = $join->left_table;
      $left_field = $join->left_field;
      $base_field = $join->base_field;
      $handler = $join->handler;
      
      // if the 'node' table is in our integrated list then
      // we want to skip it. It shouldn't be there.
      if ($left_table == 'node') {
      	continue;
      }

      // add join entry
      $data[$base_table]['table']['join'][$left_table] = array(
        'left_field' => $left_field,
        'field' => $base_field,
      );
      if ($handler) {
        $data[$base_table]['table']['join'][$left_table]['handler'] = $handler;
      }
    }
  }
  return $data;
}

/**
 *
 * @ingroup tripal_views
 */
function tripal_views_views_data_alter(&$data) {
    $tvi_query = db_query('SELECT * FROM {tripal_views}');

    // iterate through the views that we manage
    while ($tvi_row = db_fetch_object($tvi_query)) {

      //ids we'll use for queries
      $mview_id = $tvi_row->mview_id;
      $setup_id = $tvi_row->setup_id;
      

      // iterate through the columns and alter the existing data array for
      // joins to other tables
      $sql = "SELECT * FROM {tripal_views_join} WHERE setup_id = %d";
      $joins = db_query($sql, $setup_id);
      while ($join = db_fetch_object($joins)) {
        $left_table = $join->left_table;
        $left_field = $join->left_field;
        $base_field = $join->base_field;
        $base_table = $join->base_table;

        // add the recipricol join entries for each column
        if (array_key_exists($left_table, $data) and $base_table != 'node') {
          $data[$left_table]['table']['join'][$base_table] = array(
            'left_field' => $base_field,
            'field' => $left_field,
          );
        }
      }
      
      // add in joins to the node tables if the Chado schema is local
      if (tripal_core_chado_schema_exists()) {
        $base_table = $tvi_row->table_name;
        $linker_table = 'chado_' . $base_table;
        // if a node linking table exists then add in the joins
        if (db_table_exists($linker_table)) {         
      
          $data['node']['table']['join'][$linker_table] = array(
            'left_field' => 'nid',
            'field' => 'nid',
          );
          $data['node']['table']['join'][$base_table] = array(
            'left_table' => $linker_table,
            'left_field' => 'nid',
            'field' => 'nid',
          );
        }
      }
    }     
    return $data;
}

/**
 * Implementation of hook_views_plugins().
 */
function tripal_views_views_plugins() {
  $tc_path = drupal_get_path('module', 'tripal_views');

  $style_defaults = array(
    'path' => $tc_path . '/views_data_export/plugins',
    'parent' => 'views_data_export',
    'theme' => 'views_data_export',
    'theme path' => $tc_path . '/views_data_export/theme',
    'theme file' => 'tripal_views_data_export.theme.inc',
    'uses row plugin' => FALSE,
    'uses fields' => TRUE,
    'uses options' => TRUE,
    'type' => 'data_export',
  );

  // add FASTA format as a new style for the existing views_export_data Display
  return array(
    'style' => array(
      'views_data_export_fasta' => array(
        'title' => t('FASTA file'),
        'help' => t('Display results in FASTA format. All fields in results are on the definition line while the feature.residues field must be present .'),
        'handler' => 'tripal_views_plugin_style_export_fasta',
        // Views Data Export element that will be used to set additional headers when serving the feed.
        'export headers' => array('Content-type: text/plain; charset=utf-8'),
        // Views Data Export element mostly used for creating some additional classes and template names.
        'export feed type' => 'fasta',
        'export feed text' => 'FASTA',
        'export feed file' => '%view.fna',
        'export feed icon' => $tc_path . '/views_data_export/images/fasta.png',
        'additional themes' => array(
          'views_data_export_fasta_header' => 'style',
          'views_data_export_fasta_body' => 'style',
          'views_data_export_fasta_footer' => 'style',
        ),
        'additional themes base' => 'views_data_export_fasta',
      ) + $style_defaults,
    ),
  );
}

/**
 * Implementation of hook_views_pre_view().
 */
function tripal_views_views_pre_view(&$view, &$display_id, &$args) {

  // merge the $_GET and $_POST into the $_GET. This is because
  // Views and Views Data Export modules only uses the $_GET variable but
  // file uploads require $_POST. We need to make sure these two modules
  // have access to everything needed for this view to work properlys
  $_GET = array_merge($_GET, $_POST);
}

/**
 * Implementation of hook_views_pre_build().
 */
/* function tripal_views_views_pre_render(&$view, &$display_id, &$args){
  // we want to add to the bottom of the views the form for downloading
  // results in other formats (e.g. Excel, FASTA, CSV, etc.).  The Views Data
  // Export module provides small images at the bottom, but we want to provide
  // a more intutitive interface for getting different file formats
  $form = drupal_get_form('tripal_views_data_export_download_form',$view,$display_id,$args);
  $view->attachment_after = $form;
}*/

/*
 *

function tripal_views_views_query_alter(&$view, &$query){
  // iterate through the tables and see if they are chado tables.
  // if they are then prefix them with a "chado." prefix
  dpm($query);
  $tables = $query->tables;
  foreach ($tables as $base => $subtables) {
    $desc = tripal_core_get_chado_table_schema($base);
    if ($desc) {
      $query->tables["chado." . $base] = $subtables;
      unset($query->tables[$base]);
      $base = "chado." . $base;
    }
    foreach ($subtables as $subtable => $values) {
      $desc = tripal_core_get_chado_table_schema($subtable);
      if ($desc) {
        $query->tables[$base]["chado." . $subtable] = $values;
        unset($query->tables[$base][$subtable]);
      }
    }
  }

  dpm($query);
} */
/**
 * Implementation of hook_views_pre_execute().
 */
function tripal_views_views_pre_execute(&$view) {
  // if the base table is a chado table then we want to set the
  // search path so it can find all of the tables.
  // this will break views that use tables that have the same name
  // as chado tables (e.g. contact).
  $desc = tripal_core_get_chado_table_schema($view->base_table);
  if ($desc) {
    tripal_db_set_chado_search_path('chado');
  }
}
/**
 * Implementation of hook_views_post_render().
 */
function tripal_views_views_post_render(&$view, &$output, &$cache) {
  // if the base table and the query is completed we want to set
  // the search path back to the default.
  $desc = tripal_core_get_chado_table_schema($view->base_table);
  if ($desc) {
    tripal_db_set_default_search_path();
  }
}

/**
 * the Download Views data export form
 */
function tripal_views_data_export_download_form(&$form_state, $view, $display_id, $args) {
  $form = array();
  $urls = array();

  // get any export_data_export displays
  $displays = $view->display;
  $options = array();
  $default = '';
  $current_display = $view->current_display;
  foreach ($displays as $name => $display) {
    if (preg_match("/^views_data_export/", $name)) {

      // only add this display to the form if it is attached
      $display_options = $display->display_options;
      if (strcmp($display_options['displays'][$current_display], $current_display) != 0) {
         continue;
      }

      // set the first item as default
      if (!$default) {
        $default = $display->id;
      }

      $path = $display->display_options['path'];
      $query = $view->get_exposed_input();  // retrieves elements in $_GET array

      $urls[$display->id]['path'] = $path;
      $urls[$display->id]['query'] = $query;

      // add the new item to the options array
      $options[$display->id] = $display->display_title;
    }
  }

  // only generate the form if we have views_data_export displays attached
  // to this view
  if (count($options) > 0) {

    $form_state['storage']['urls'] = $urls;
    $form['#cache'] = TRUE;

    // we want the form to redirect to a new window
    $form['#attributes']['target'] = "_blank";

    // now build the form elements
    $form['downloads'] = array(
      '#type' => 'fieldset',
      '#title' => 'Download Results',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE
    );
    $form['downloads']['file_type'] = array(
      '#title' => t('File format'),
      '#type' => 'radios',
      '#options' => $options,
      '#required' => TRUE,
      '#default_value' => $default,
      '#description' => t('Please select a file format to download'),
    );
    $form['downloads']['submit'] = array(
      '#value' => t('Download Results'),
      '#type' => 'submit',
    );
  }
  return $form;
}

/**
 * Submit for the Download Views data export form
 */
function tripal_views_data_export_download_form_submit($form, &$form_state) {
  $urls = $form_state['storage']['urls'];
  $display_id = $form_state['values']['file_type'];
  drupal_goto($urls[$display_id]['path'], $urls[$display_id]['query']);
}




