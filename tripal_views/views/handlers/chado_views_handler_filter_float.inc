<?php

/**
 * @file
 * A chado wrapper for the views_handler_filter_float.
 *
 * Handles fields which may be aggregated during the chado join process. There are options
 * to filter the base table based on an aggregated table or just filter the aggregated
 * table (showing blank for that field if there are no records in the aggregated table).
 */
class chado_views_handler_filter_float extends views_handler_filter_float {

  /**
   * Defines the options form (form available to admin when they add a field to a view)
   */
  function options_form(&$form, &$form_state) {
    $form['msg'] = array(
      '#type' => 'item',
      '#value' => '<b>If this filter applies to a table that is aggregated, additionally options may be ignored.</b>'
    );

    parent::options_form($form, $form_state);

    $form['agg'] = array(
      '#type' => 'fieldset',
      '#title' => 'Apply to fields that are aggregated'
    );

    $form['agg']['records_with'] = array(
      '#type' => 'checkbox',
      '#title' => t('Filter base table records'),
      '#description' => t('Filters %base_table to only those with the value in the aggregate array.', array('%base_table' => $this->view->base_table)),
      '#default_value' => (isset($this->options['agg']['records_with'])) ? $this->options['agg']['records_with'] : TRUE,
    );

    $form['agg']['aggregates_with'] = array(
      '#type' => 'checkbox',
      '#title' => t('Filter aggregates displayed'),
      '#description' => t('Filters the aggregates shown based on the value. Doesn\'t affect the number of %base_table records.', array('%base_table' => $this->view->base_table)),
      '#default_value' => (isset($this->options['agg']['aggregates_with'])) ? $this->options['agg']['aggregates_with'] : TRUE,
    );

  }

  /**
   * If the table to be filtered is not aggregated uses the parent::query()
   * However, if it is uses postgresql any() function to compare
   */
  function query() {

    // make optional
    // if it is not set or empty then don't restrict the query
    if (!$this->value AND !is_array($this->value)) {
      return;
    }
    elseif (is_array($this->value)) {
      $tmp = array_filter($this->value);
      if (!$tmp) {
        return;
      }
    }

    $this->ensure_my_table();

    $table = $this->query->get_table_info($this->table);
    if (preg_match('/aggregator/', $table['join']->definition['handler'])) {
      $this->aggregated = TRUE;
    }
    else {
      $this->aggregated = FALSE;
    }

    if (!$this->aggregated) {
      parent::query();
    }
    else {

      // Only base records with value in the aggregated field
      // This doesn't restrict the items in the aggregate field
      $this->ensure_my_table();
      $field = "$this->table_alias.$this->real_field";
      if ($this->options['agg']['records_with']) {
        $where = "'%s' = ANY($field)";
        $this->query->add_where($this->options['group'], $where, $this->value);
      }

      // To restrict the items in the aggregate...
      // Tell the join handler about the filter
      // so it can be done in the join query
      if ($this->options['agg']['aggregates_with']) {
        $table['join']->filter[] = $field . " = '" . $this->value . "'";
      }
    }

  }

}